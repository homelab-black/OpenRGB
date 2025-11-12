dnf install rpmdevtools dnf-plugins-core libcurl-devel qt5-qtbase-devel git -y
rpmdev-setuptree
./scripts/build-package-files.sh fedora/OpenRGB.spec
cp fedora/OpenRGB.spec /root/rpmbuild/SPECS
cp -rp . /root/rpmbuild/SOURCES/OpenRGB
cd /root/rpmbuild
dnf builddep SPECS/OpenRGB.spec -y
rpmbuild -ba SPECS/OpenRGB.spec
cd RPMS/x86_64/
# mv openrgb*.rpm ${CI_PROJECT_DIR}/
# cd ${CI_PROJECT_DIR}
