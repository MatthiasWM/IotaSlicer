//
// IAVersioneer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAVersioneer.h"

#include <errno.h>


#ifdef _WIN32
FILE *popen(const char *name, const char *mode) {
	return _popen(name, mode);
}
void pclose(FILE *f) {
	_pclose(f);
}

#include <windows.h> 
#include <stdio.h>
#include <malloc.h>

#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C } // extern "C"
#define null nullptr
#else
#define BEGIN_C
#define END_C
#define null ((void*)0)
#endif


int system_np(const char* command, int timeout_milliseconds,
	char* stdout_data, int stdout_data_size,
	char* stderr_data, int stderr_data_size, int* exit_code);

typedef struct system_np_s {
	HANDLE child_stdout_read;
	HANDLE child_stderr_read;
	HANDLE reader;
	PROCESS_INFORMATION pi;
	const char* command;
	char* stdout_data;
	int   stdout_data_size;
	char* stderr_data;
	int   stderr_data_size;
	int*  exit_code;
	int   timeout; // timeout in milliseconds or -1 for INIFINTE
} system_np_t;

static char stdout_data[16 * 1024 * 1024];
static char stderr_data[16 * 1024 * 1024];

int prun(const char *cmd) {
	stdout_data[0] = 0;
	stderr_data[0] = 0;
	char* command = strdup(cmd);
	int exit_code = 0;
	int r = system_np(command, 5 * 1000, stdout_data, sizeof(stdout_data), stderr_data, sizeof(stderr_data), &exit_code);
	if (r != 0) {
		fprintf(stderr, "system_np failed: %d 0x%08x %s", r, r, strerror(r));
		return r;
	}
	else {
		fwrite(stdout_data, strlen(stdout_data), 1, stdout);
		fwrite(stderr_data, strlen(stderr_data), 1, stderr);
		return exit_code;
	}
}

static int peek_pipe(HANDLE pipe, char* data, int size) {
	char buffer[4 * 1024];
	DWORD read = 0;
	DWORD available = 0;
	bool b = PeekNamedPipe(pipe, null, sizeof(data), null, &available, null);
	if (!b) {
		return -1;
	}
	else if (available > 0) {
		int bytes = min(sizeof(buffer), available);
		b = ReadFile(pipe, buffer, bytes, &read, null);
		if (!b) {
			return -1;
		}
		if (data != null && size > 0) {
			int n = min(size - 1, (int)read);
			memcpy(data, buffer, n);
			data[n + 1] = 0; // always zero terminated
			return n;
		}
	}
	return 0;
}

static DWORD WINAPI read_from_all_pipes_fully(void* p) {
	system_np_t* system = (system_np_t*)p;
	unsigned long long milliseconds = GetTickCount64(); // since boot time
	char* out = system->stdout_data != null && system->stdout_data_size > 0 ? system->stdout_data : null;
	char* err = system->stderr_data != null && system->stderr_data_size > 0 ? system->stderr_data : null;
	int out_bytes = system->stdout_data != null && system->stdout_data_size > 0 ? system->stdout_data_size - 1 : 0;
	int err_bytes = system->stderr_data != null && system->stderr_data_size > 0 ? system->stderr_data_size - 1 : 0;
	for (;;) {
		int read_stdout = peek_pipe(system->child_stdout_read, out, out_bytes);
		if (read_stdout > 0 && out != null) { out += read_stdout; out_bytes -= read_stdout; }
		int read_stderr = peek_pipe(system->child_stderr_read, err, err_bytes);
		if (read_stderr > 0 && err != null) { err += read_stderr; err_bytes -= read_stderr; }
		if (read_stdout < 0 && read_stderr < 0) { break; } // both pipes are closed
		unsigned long long time_spent_in_milliseconds = GetTickCount64() - milliseconds;
		if (system->timeout > 0 && time_spent_in_milliseconds > system->timeout) { break; }
		if (read_stdout == 0 && read_stderr == 0) { // nothing has been read from both pipes
			HANDLE handles[2] = { system->child_stdout_read, system->child_stderr_read };
			WaitForMultipleObjects(2, handles, false, 1); // wait for at least 1 millisecond (more likely 16)
		}
	}
	if (out != null) { *out = 0; }
	if (err != null) { *err = 0; }
	return 0;
}

static int create_child_process(system_np_t* system) {
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = true;
	sa.lpSecurityDescriptor = null;
	HANDLE child_stdout_write = INVALID_HANDLE_VALUE;
	HANDLE child_stderr_write = INVALID_HANDLE_VALUE;
	if (!CreatePipe(&system->child_stderr_read, &child_stderr_write, &sa, 0)) {
		return GetLastError();
	}
	if (!SetHandleInformation(system->child_stderr_read, HANDLE_FLAG_INHERIT, 0)) {
		return GetLastError();
	}
	if (!CreatePipe(&system->child_stdout_read, &child_stdout_write, &sa, 0)) {
		return GetLastError();
	}
	if (!SetHandleInformation(system->child_stdout_read, HANDLE_FLAG_INHERIT, 0)) {
		return GetLastError();
	}
	// Set the text I want to run
	STARTUPINFOA siStartInfo = { 0 };
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = child_stderr_write;
	siStartInfo.hStdOutput = child_stdout_write;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;
	bool b = CreateProcessA(null,
		(char*)system->command,
		null,               // process security attributes 
		null,               // primary thread security attributes 
		true,               // handles are inherited 
		CREATE_NO_WINDOW,   // creation flags 
		null,               // use parent's environment 
		null,               // use parent's current directory 
		&siStartInfo,       // STARTUPINFO pointer 
		&system->pi);       // receives PROCESS_INFORMATION
	int err = GetLastError();
	CloseHandle(child_stderr_write);
	CloseHandle(child_stdout_write);
	if (!b) {
		CloseHandle(system->child_stdout_read); system->child_stdout_read = INVALID_HANDLE_VALUE;
		CloseHandle(system->child_stderr_read); system->child_stderr_read = INVALID_HANDLE_VALUE;
	}
	return b ? 0 : err;
}

int system_np(const char* command, int timeout_milliseconds,
	char* stdout_data, int stdout_data_size,
	char* stderr_data, int stderr_data_size, int* exit_code) {
	system_np_t system = { 0 };
	if (exit_code != null) { *exit_code = 0; }
	if (stdout_data != null && stdout_data_size > 0) { stdout_data[0] = 0; }
	if (stderr_data != null && stderr_data_size > 0) { stderr_data[0] = 0; }
	system.timeout = timeout_milliseconds > 0 ? timeout_milliseconds : -1;
	system.command = command;
	system.stdout_data = stdout_data;
	system.stderr_data = stderr_data;
	system.stdout_data_size = stdout_data_size;
	system.stderr_data_size = stderr_data_size;
	int r = create_child_process(&system);
	if (r == 0) {
		system.reader = CreateThread(null, 0, read_from_all_pipes_fully, &system, 0, null);
		if (system.reader == null) { // in theory should rarely happen only when system super low on resources
			r = GetLastError();
			TerminateProcess(system.pi.hProcess, ECANCELED);
		}
		else {
			bool thread_done = WaitForSingleObject(system.pi.hThread, timeout_milliseconds) == 0;
			bool process_done = WaitForSingleObject(system.pi.hProcess, timeout_milliseconds) == 0;
			if (!thread_done || !process_done) {
				TerminateProcess(system.pi.hProcess, ETIME);
			}
			if (exit_code != null) {
				GetExitCodeProcess(system.pi.hProcess, (DWORD*)exit_code);
			}
			CloseHandle(system.pi.hThread);
			CloseHandle(system.pi.hProcess);
			CloseHandle(system.child_stdout_read); system.child_stdout_read = INVALID_HANDLE_VALUE;
			CloseHandle(system.child_stderr_read); system.child_stderr_read = INVALID_HANDLE_VALUE;
			WaitForSingleObject(system.reader, INFINITE); // join thread
			CloseHandle(system.reader);
		}
	}
	if (stdout_data != null && stdout_data_size > 0) { stdout_data[stdout_data_size - 1] = 0; }
	if (stderr_data != null && stderr_data_size > 0) { stderr_data[stderr_data_size - 1] = 0; }
	return r;
}

#endif


/**
 * \todo copy newly built FLTK binaries automatically:
 *      /Users/matt/dev/fltk-1.4.svn/build/Xcode/bin/Release/fluid.app
 * \todo add terminal view when running system commands to see output
 * \todo path setting and "release" button to create distributables
 */

void IAVersioneer::loadSettings()
{
    int v;
    char buf[2048];

    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.get("path", buf, "/Users/matt/dev/IotaSlicer/", 2046);
    wBasePath->value(buf);
    pref.get("archivePath", buf, "/Users/matt/Desktop/", 2046);
    wArchivePath->value(buf);
    pref.get("FLTKPath", buf, "/Users/matt/dev/fltk-1.4.svn", 2046);
    wFLTKPath->value(buf);
    Fl_Preferences vers( pref, "version");
    vers.get("major", v, 0);
    sprintf(buf, "%d", v);
    wMajor->value(buf);
    vers.get("minor", v, 0);
    sprintf(buf, "%d", v);
    wMinor->value(buf);
    vers.get("build", v, 0);
    sprintf(buf, "%d", v);
    wBuild->value(buf);
    vers.get("class", buf, "-", 8);
    wClass->value(buf);

    const char *name = "README.md";

    char filename[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    FILE *f = fopen(filename, "rb");
    if (f) {
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "<!--[ver-->");
            char *b = strstr(buf, "<!--]-->");
            if (a!=nullptr && b!=nullptr) {
                *b = 0;
                int maj, min, bld;
                char cls[80];
                int cc = sscanf(a+11, "v%d.%d.%d%s", &maj, &min, &bld, cls);
                if (cc>0) {
                    sprintf(buf, "%d", maj);
                    wMajor->value(buf);
                }
                if (cc>1) {
                    sprintf(buf, "%d", min);
                    wMinor->value(buf);
                }
                if (cc>2) {
                    sprintf(buf, "%d", bld);
                    wBuild->value(buf);
                }
                if (cc>3) {
                    wClass->value(cls);
                }
                break;
            }
        }
        fclose(f);
        wTerminal->printf("Versioneer: found current version in %s\n", filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::saveSettings()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("path", wBasePath->value());
    pref.set("archivePath", wArchivePath->value());
    pref.set("FLTKPath", wFLTKPath->value());
    Fl_Preferences vers( pref, "version");
    vers.set("major", atoi(wMajor->value()));
    vers.set("minor", atoi(wMinor->value()));
    vers.set("build", atoi(wBuild->value()));
    vers.set("class", wClass->value());
}

void IAVersioneer::setProjectPath()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("path", wBasePath->value());
}

void IAVersioneer::selectProjectPath()
{
    const char *path = fl_dir_chooser("Choose the Iota Base Directory",
                                      wBasePath->value(),
                                      0);
    if (path) {
        wBasePath->value(path);
        wBasePath->do_callback();
    }
}


void IAVersioneer::setFLTKPath()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("FLTKPath", wFLTKPath->value());
}


void IAVersioneer::selectFLTKPath()
{
    const char *path = fl_dir_chooser("Choose the FLTK root Directory",
                                      wFLTKPath->value(),
                                      0);
    if (path) {
        wFLTKPath->value(path);
        wFLTKPath->do_callback();
    }
}


void IAVersioneer::updateFLTK()
{
    if (fl_choice(
                  "Did you build all FLTK libraries and Fluid\n"
                  "in Debug *and* Release mode?\n\n"
                  "Set MacOS Deployment Target to 10.9!",
                  "Cancel", "Continue", nullptr) == 0)
        return;

    fl_chdir(wBasePath->value());
#ifdef __APPLE__
    system("mkdir platforms");
    system("mkdir platforms/MacOS");
    system("mkdir platforms/MacOS/bin");
    systemf("cp %s/build/Xcode/bin/Debug/fluid.app/Contents/MacOS/fluid platforms/MacOS/bin",
            wFLTKPath->value());
    // TODO: separate debug and release builds
    systemf("cp %s/build/Xcode/lib/Debug/lib* platforms/MacOS/lib",
            wFLTKPath->value());
    system("mkdir include");
    systemf("cp -R %s/FL include", wFLTKPath->value());
    systemf("cp %s/build/Xcode/FL/abi-version.h include/FL", wFLTKPath->value());
    system("mkdir include/libjpeg");
    system("mkdir include/libpng");
    systemf("cp %s/jpeg/*.h include/libjpeg/", wFLTKPath->value());
    systemf("cp %s/jpeg/*.h include/libjpeg/", wFLTKPath->value());
    systemf("cp %s/png/*.h include/libpng/", wFLTKPath->value());
#endif
#ifdef _WIN32
	system("cmd \\c echo \"Not implemented on MSWindows\"");
#if 0
	system("cmd \\c md platforms");
	system("cmd \\c md platforms/MSWindows");
	system("cmd \\c md platforms/MSWindows/bin");
	systemf("xcopy %s/build/VisualC/bin/Debug/fluid.exe platforms/MSWindows/bin",
		wFLTKPath->value());
	// TODO: separate debug and release builds
	systemf("xcopy %s/build/VisualC/lib/Release/lib* platforms/MSWindows/lib \\s \\e \\y",
		wFLTKPath->value());
	system("cmd \\c md include");
	systemf("xcopy %s/FL include \\s \\e \\y", wFLTKPath->value());
	systemf("xcopy %s/build/VisualC/FL/abi-version.h include/FL \\s \\e \\y", wFLTKPath->value());
	system("cmd \\c md include/libjpeg");
	system("cmd \\c md include/libpng");
	systemf("xcopy %s/jpeg/*.h include/libjpeg/ \\s \\e \\y", wFLTKPath->value());
	systemf("xcopy %s/jpeg/*.h include/libjpeg/ \\s \\e \\y", wFLTKPath->value());
	systemf("xcopy %s/png/*.h include/libpng/ \\s \\e \\y", wFLTKPath->value());
#endif
#endif
#ifdef __LINUX__
#endif
}


void IAVersioneer::setArchivePath()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("archivePath", wArchivePath->value());
}

void IAVersioneer::selectArchivePath()
{
    const char *path = fl_dir_chooser("Choose the Archive Directory",
                                      wArchivePath->value(),
                                      0);
    if (path) {
        wArchivePath->value(path);
        wArchivePath->do_callback();
    }
}


void IAVersioneer::createArchive()
{
#ifdef __APPLE__
    // in platforms/MacOS
    // xcodebuild -target IotaSlicer -configuration Release
    // xcodebuild -scheme Iota -showBuildSettings | grep TARGET_BUILD_DIR
    fl_message("Select the menu 'Product > Archive' in Xcode.\nArchive will be created in $HOME/Desktop");
#endif
#ifdef _WIN32
	if (fl_choice(
		"Select 'Release' Solution Configuration in VisualC and press F7.\n"
		"Archive will be created in the given directory.", "Cancel", "Continue", nullptr) == 1)
	{
		char buf[4096];
		sprintf(buf,
			"cmd \\c cd %s/platforms/MSWindows/VisualStudio/Release && "
			"powershell Compress-Archive "
				"-Path IotaSlicer.exe "
				"-DestinationPath %s/IotaSlicer_%d.%d.%d%s_MSWindows.zip "
				"-Force",
			wBasePath->value(),
			wArchivePath->value(),
			atoi(wMajor->value()),
			atoi(wMinor->value()),
			atoi(wBuild->value()),
			wClass->value()
			);
		system(buf);
	}
#endif
#ifdef __LINUX__
	if (fl_choice(
		"Select 'Release' in CodeBlock and click 'Build'.\n"
		"Archive will be created in the given directory.", "Cancel", "Continue", nullptr) == 1)
	{
		char buf[4096];
		sprintf(buf,
			"cd %s/platforms/Linux/CodeBlocks/bin/Release/ && "
			"/usr/bin/zip -r -y "
				"%s/IotaSlicer_%d.%d.%d%s_Linux.zip "
				"IotaSlicer ",
			wBasePath->value(),
			wArchivePath->value(),
			atoi(wMajor->value()),
			atoi(wMinor->value()),
			atoi(wBuild->value()),
			wClass->value()
			);
		system(buf);
	}
#endif
}


void IAVersioneer::majorPlus()
{
    char buf[80];
    int vv = atoi(wMajor->value());
    vv++;
    sprintf(buf, "%d", vv);
    wMajor->value(buf);
    wMinor->value("0");
    wBuild->value("0");
}


void IAVersioneer::minorPlus()
{
    char buf[80];
    int vv = atoi(wMinor->value());
    vv++;
    sprintf(buf, "%d", vv);
    wMinor->value(buf);
    wBuild->value("0");
}


void IAVersioneer::buildPlus()
{
    char buf[80];
    int vv = atoi(wBuild->value());
    vv++;
    sprintf(buf, "%d", vv);
    wBuild->value(buf);
}


void IAVersioneer::applySettingsHtml(const char *name) {
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "<!--[ver-->");
            char *b = strstr(buf, "<!--]-->");
            if (a!=nullptr && b!=nullptr) {
                char *c = strdup(b);
                sprintf(a,
                        "<!--[ver-->v%d.%d.%d%s%s",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value(),
                        c
                        );
                free(c);
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        wTerminal->printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::applySettingsCpp(const char *name)
{
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "/*[ver*/");
            char *b = strstr(buf, "/*]*/");
            if (a!=nullptr && b!=nullptr) {
                char *c = strdup(b);
                sprintf(a,
                        "/*[ver*/\"v%d.%d.%d%s\"%s",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value(),
                        c
                        );
                free(c);
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        wTerminal->printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::applySettingsDoxyfile(const char *name)
{
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            if (strncmp(buf, "PROJECT_NUMBER         = ", 25)==0) {
                sprintf(buf,
                        "PROJECT_NUMBER         = %d.%d.%d%s\n",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value()
                        );
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        wTerminal->printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}


void IAVersioneer::touch(const char *name)
{
    char filename[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    ::utime(filename, nullptr);
}


void IAVersioneer::updatePlatformFiles()
{
#ifdef __APPLE__
    char rev[80];
    strcpy(rev, "0000000");
    FILE *f = ::popen("/usr/bin/git rev-parse --short HEAD", "r");
    fgets(rev, 78, f);
    ::pclose(f);
    rev[7] = 0;

    char filename[2048];
    sprintf(filename, "%s/platforms/MacOS/Info.plist", wBasePath->value());
    char cmd[2*2048];
    sprintf(cmd,
            "/usr/libexec/PlistBuddy -c \"Set :CFBundleVersion %s\" %s",
            rev, filename);
    system(cmd);
    sprintf(cmd,
            "/usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString %d.%d.%d%s\" %s",
            atoi(wMajor->value()),
            atoi(wMinor->value()),
            atoi(wBuild->value()),
            wClass->value(),
            filename);
    system(cmd);
#endif
}


void IAVersioneer::applySettings()
{
    fl_chdir(wBasePath->value());
    applySettingsHtml("README.md");
    applySettingsHtml("html/helpAbout.html");
    applySettingsHtml("html/helpLicenses.html");
    applySettingsHtml("html/index.html");
    applySettingsCpp("src/Iota.cpp");
    applySettingsDoxyfile("Doxyfile");
    updatePlatformFiles();
    touch("view/IAGUIMain.fl");
}


void IAVersioneer::gitTag()
{
#ifdef _WIN32
	system("echo \"Iota gitTag not available on MSWindows");
#else
	char buf[2048];
    sprintf(buf, "cd %s; /usr/bin/git tag v%d.%d.%d%s",
            wBasePath->value(),
            atoi(wMajor->value()),
            atoi(wMinor->value()),
            atoi(wBuild->value()),
            wClass->value());
    system(buf);
    sprintf(buf, "cd %s; /usr/bin/git push --tags",
            wBasePath->value());
    system(buf);
#endif
}


IAVersioneer::IAVersioneer()
{
}


IAVersioneer::~IAVersioneer()
{
}


void IAVersioneer::systemf(const char *fmt, ...)
{
    char buf[4*FL_PATH_MAX];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    system(buf);
}

void IAVersioneer::system(const char *cmd)
{
    wTerminal->printf("> %s\n", cmd);
#ifdef _WIN32
	char *cmdDOS = strdup(cmd);
	char *p = cmdDOS;
	while (*p) {
		if (*p == '/') *p = '\\';
		else if (*p == '\\') *p = '/';
		p++;
	}
	int ret = prun(cmdDOS);
	wTerminal->printf("Returned: %d\n", ret);
	wTerminal->printf("%s", stdout_data);
	wTerminal->printf("%s", stderr_data);
	free(cmdDOS);
#else
    FILE *f = ::popen(cmd, "r");
    if (f) {
        while (!::feof(f)) {
            char buf[82];
            if (fgets(buf, 80, f)) {
                wTerminal->append(buf);
            }
        }
        ::pclose(f);
    } else {
        wTerminal->printf("Failed: %s\n", strerror(errno));
    }
#endif
}


void IAVersioneer::show()
{
    if (!pDialog) {
        pDialog = createWindow();
        wTerminal->ansi(true);
        wTerminal->color(FL_BLACK);
    }
    if (pDialog) {
        loadSettings();
        pDialog->show();
    }
}



