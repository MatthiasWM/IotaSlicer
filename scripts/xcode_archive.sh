
# find the version string to crea a named archive
export IA_VERSION=`/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" "${INFOPLIST_FILE}"`
echo "Create MAcOS archive for Iota Slicer ${IA_VERSION}"

# find the archived executable and zip it into an archive
export IA_ARCHIVE="${HOME}/Desktop/IotaSlicer_${IA_VERSION}_MacOS.zip"
cd "${ARCHIVE_PRODUCTS_PATH}/Applications/"
rm -f ${IA_ARCHIVE}
zip -r -y ${IA_ARCHIVE} IotaSlicer.app
