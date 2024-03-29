##
##  CmakeLists.txt
##
##  Copyright (c) 2013-2024 Matthias Melcher. All rights reserved.
##

cmake_minimum_required(VERSION 3.25.2)

project (IotaSlicer)

set (IotaSlicer_VERSION_MAJOR 0)
set (IotaSlicer_VERSION_MINOR 3)
set (IotaSlicer_VERSION_BUILD 0)

set (CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set (CMAKE_XCODE_GENERATE_SCHEME TRUE)

## ---- Download external FLTK library ----

include(FetchContent)

FetchContent_Declare(
  FLTK
  GIT_REPOSITORY  https://github.com/fltk/fltk
  GIT_TAG         master
  GIT_SHALLOW     TRUE
)

message(STATUS "Downloading and configuring FLTK if necessary, please wait...")
set( FLTK_USE_SYSTEM_LIBJPEG OFF )
set( FLTK_USE_SYSTEM_ZLIB OFF )
set( FLTK_USE_SYSTEM_LIBPNG OFF )
#set( FLTK_BUILD_FLUID ON )
set( FLTK_BUILD_TEST OFF )
set( FLTK_BUILD_GL ON )
set( FLTK_BUILD_FORMS OFF )
set( FLTK_BUILD_FLTK_OPTIONS OFF )
set( FLTK_BUILD_HTML_DOCS OFF )
set( FLTK_BUILD_PDF_DOCS OFF )
FetchContent_MakeAvailable(FLTK)
message(STATUS "Downloading and configuring FLTK - done.")

message(STATUS "Fluid: " fltk::fluid " : " ${FLTK_FLUID_EXECUTABLE} )

function(FLTK_RUN_FLUID TARGET SOURCES)
    set (FLUID_FILES ${${TARGET}})
    foreach(src ${SOURCES})
        if ("${src}" MATCHES "\\.fl$")
            string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
            get_filename_component (src_name ${src} NAME)
            get_filename_component (src_absolute ${src} ABSOLUTE)
            get_filename_component (srcdir_absolute ${src_absolute} DIRECTORY)
            add_custom_command(
                WORKING_DIRECTORY "${srcdir_absolute}"
                COMMAND fltk::fluid -c ${src_name}
                DEPENDS ${src}
                OUTPUT "${basename}.cpp"
                OUTPUT "${basename}.h"
            )
            list(APPEND FLUID_FILES "${PROJECT_SOURCE_DIR}/${basename}.cpp")
            list(APPEND FLUID_FILES "${PROJECT_SOURCE_DIR}/${basename}.h")
            list(APPEND FLUID_FILES "${PROJECT_SOURCE_DIR}/${basename}.fl")
        endif ("${src}" MATCHES "\\.fl$")
        set (${TARGET} ${FLUID_FILES} PARENT_SCOPE)
    endforeach(src)
endfunction(FLTK_RUN_FLUID TARGET SOURCES)

FLTK_RUN_FLUID (FLUID_VIEWS src/view/IAGUIMain.fl)
FLTK_RUN_FLUID (FLUID_DATA src/data/binaryData.fl)


# http://lists.qt-project.org/pipermail/qt-creator/2012-August/001191.html [^]
function(collect_info_files)
    list(APPEND _all_found)
    foreach(_it ${ARGN})
        if(NOT IS_DIRECTORY ${_it})
            get_filename_component(_path ${_it} ABSOLUTE)
            if(EXISTS ${_path})
                list(APPEND _all_found ${_it})
                if(NOT ${_it} MATCHES "^/\\\\..*$;~$")
                    set_source_files_properties(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
                endif()
            endif()
        endif()
    endforeach()
    set(INFO_FILES ${_all_found} PARENT_SCOPE)
endfunction()


set(other_files README.md Doxyfile html/index.html html/helpAbout.html html/helpLicenses.html)
collect_info_files(${other_files})
# add_custom_target(docs SOURCES ${INFO_FILES})

find_package(Doxygen)
if (DOXYGEN_FOUND)
    message("Doxygen found: ${DOXYGEN_EXECUTABLE}")
#    add_custom_target( iota_docs ALL
#        COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
#        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#        COMMENT "Generating API documentation with Doxygen"
#        VERBATIM
#    )
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

add_executable (IotaSlicer MACOSX_BUNDLE
	src/Iota.cpp
	src/Iota.h
	src/app/IAError.cpp
	src/app/IAError.h
	src/app/IAMacros.h
	src/app/IAPreferences.cpp
	src/app/IAPreferences.h
	src/app/IAVersioneer.cpp
	src/app/IAVersioneer.h
	src/controller/IAController.cpp
	src/controller/IAController.h
	src/controller/IAPrinterListController.cpp
	src/controller/IAPrinterListController.h
    ${FLUID_DATA}
	src/fileformats/IAFmtObj3ds.cpp
	src/fileformats/IAFmtObj3ds.h
	src/fileformats/IAFmtTexJpeg.cpp
	src/fileformats/IAFmtTexJpeg.h
	src/fileformats/IAGeometryReader.cpp
	src/fileformats/IAGeometryReader.h
	src/fileformats/IAGeometryReaderBinaryStl.cpp
	src/fileformats/IAGeometryReaderBinaryStl.h
	src/fileformats/IAGeometryReaderTextStl.cpp
	src/fileformats/IAGeometryReaderTextStl.h
	src/geometry/IAEdge.cpp
	src/geometry/IAEdge.h
	src/geometry/IAMath.cpp
	src/geometry/IAMath.h
	src/geometry/IAMesh.cpp
	src/geometry/IAMesh.h
	src/geometry/IAMeshSlice.cpp
	src/geometry/IAMeshSlice.h
	src/geometry/IATriangle.cpp
	src/geometry/IATriangle.h
	src/geometry/IAVector3d.cpp
	src/geometry/IAVector3d.h
	src/geometry/IAVertex.cpp
	src/geometry/IAVertex.h
#    src/lua/IALua.cpp
#    src/lua/IALua.h
	src/opengl/IAFramebuffer.cpp
	src/opengl/IAFramebuffer.h
	src/potrace/IAPotrace.cpp
	src/potrace/IAPotrace.h
	src/potrace/auxiliary.h
	src/potrace/bitmap.h
	src/potrace/config.h
	src/potrace/curve.c
	src/potrace/curve.h
	src/potrace/decompose.c
	src/potrace/decompose.h
	src/potrace/lists.h
	src/potrace/platform.h
	src/potrace/potracelib.c
	src/potrace/potracelib.h
	src/potrace/progress.h
	src/potrace/trace.c
	src/potrace/trace.h
	src/printer/IAPrinter.cpp
	src/printer/IAPrinter.h
	src/printer/IAFDMPrinter.cpp
	src/printer/IAFDMPrinter.h
	src/printer/IAPrinterFDMBelt.cpp
	src/printer/IAPrinterFDMBelt.h
	src/printer/IAPrinterInkjet.cpp
	src/printer/IAPrinterInkjet.h
	src/printer/IAPrinterLasercutter.cpp
	src/printer/IAPrinterLasercutter.h
	src/printer/IAPrinterList.cpp
	src/printer/IAPrinterList.h
	src/printer/IAPrinterSLS.cpp
	src/printer/IAPrinterSLS.h
	src/property/IAProperty.cpp
	src/property/IAProperty.h
	src/toolpath/IADxfWriter.cpp
	src/toolpath/IADxfWriter.h
	src/toolpath/IAGcodeWriter.cpp
	src/toolpath/IAGcodeWriter.h
	src/toolpath/IAToolpath.cpp
	src/toolpath/IAToolpath.h
    ${FLUID_VIEWS}
	src/view/IAProgressDialog.cpp
	src/view/IAProgressDialog.h
    src/view/IATreeItemView.cpp
    src/view/IATreeItemView.h
	src/widget/IACamera.cpp
	src/widget/IACamera.h
	src/widget/IAGLButton.cpp
	src/widget/IAGLButton.h
    src/widget/IAGLRangeSlider.cpp
    src/widget/IAGLRangeSlider.h
	src/widget/IASceneView.cpp
	src/widget/IASceneView.h
    ${INFO_FILES}
)

source_group(src\\ src/)
source_group(src\\app src/app)
source_group(src\\controller src/controller)
source_group(src\\data src/data)
source_group(src\\fileformats src/fileformats)
source_group(src\\geometry src/geometry)
# source_group(src\\lua src/lua)
source_group(src\\opengl src/opengl)
source_group(src\\potrace src/potrace)
source_group(src\\printer src/printer)
source_group(src\\property src/property)
source_group(src\\toolpath src/toolpath)
source_group(src\\view src/view)
source_group(src\\widget src/widget)
source_group(html\\ html/)

add_dependencies ( IotaSlicer fltk::fltk fltk::fluid )

#if(MSVC)
#  target_compile_options(IotaSlicer PRIVATE /W4 /WX)
#else()
#  target_compile_options(IotaSlicer PRIVATE -Wall -Wextra -Wpedantic -Werror)
#endif()

find_package(OpenGL REQUIRED)
target_include_directories(IotaSlicer PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(IotaSlicer ${OPENGL_LIBRARIES})

#target_link_directories (
#  IotaSlicer
#)

target_include_directories (
  IotaSlicer PRIVATE
	src/
  ${fltk_BINARY_DIR} ${fltk_SOURCE_DIR}
)

target_link_libraries (IotaSlicer 
#	${CMAKE_SOURCE_DIR}/fltk/build/lib/libfltk.a
#	${CMAKE_SOURCE_DIR}/fltk/build/lib/libfltk_gl.a
#  fltk::fltk
  fltk::gl
  fltk::images
  fltk::jpeg
  fltk::png
  fltk::z

#	fltk
#	fltk_gl
#	fltk_images
#	fltk_jpeg
#	fltk_png
#	fltk_z
#    lua543
#    lua53
#	3ds
)


# APPLE WIN32 MSVC UNIX
if (APPLE)
# set( OSX_ICON_FILES vvv.icns vvv-document.icns )
  set_target_properties ( IotaSlicer
    PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/platforms/MacOS/Info.plist
  )
#  target_link_libraries (
#    IotaSlicer
#    "-framework Cocoa"
#		"-framework AppKit"
#		"-framework CoreText"
#		"-framework CoreGraphics"
#		"-framework CoreFoundation"
#		"-framework OpenGL"
#	)
endif()
if (UNIX AND NOT APPLE)
	target_compile_definitions(IotaSlicer PUBLIC __LINUX__)
	target_compile_definitions(IotaSlicer PUBLIC GL_GLEXT_PROTOTYPES)
#	find_package(X11 REQUIRED)
#	include_directories(${X11_INCLUDE_DIR})
#	link_directories(${X11_LIBRARIES})
#	target_link_libraries(IotaSlicer ${X11_LIBRARIES})
#	target_link_libraries(IotaSlicer X11)
	target_link_libraries(IotaSlicer Xext)
#	target_link_libraries(IotaSlicer pthread)
#	target_link_libraries(IotaSlicer Xfixes)
#	target_link_libraries(IotaSlicer Xft)
#	target_link_libraries(IotaSlicer Xrender)
#	target_link_libraries(IotaSlicer m)
#	target_link_libraries(IotaSlicer fontconfig)
#	target_link_libraries(IotaSlicer dl)
endif()
if(WIN32)
#	target_link_libraries(IotaSlicer ws2_32)
#	target_link_libraries(IotaSlicer kernel32)
#	target_link_libraries(IotaSlicer user32)
#	target_link_libraries(IotaSlicer gdi32)
#	target_link_libraries(IotaSlicer winspool)
#	target_link_libraries(IotaSlicer comdlg32)
#	target_link_libraries(IotaSlicer advapi32)
#	target_link_libraries(IotaSlicer shell32)
#	target_link_libraries(IotaSlicer ole32)
#	target_link_libraries(IotaSlicer oleaut32)
#	target_link_libraries(IotaSlicer uuid)
#	target_link_libraries(IotaSlicer odbc32)
#	target_link_libraries(IotaSlicer odbccp32)
#	target_link_libraries(IotaSlicer comctl32)
endif()




function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

dump_cmake_variables ( ".*FLUID.*" )

