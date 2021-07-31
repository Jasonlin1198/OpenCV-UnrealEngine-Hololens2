# libzbar
Complete CMake Cross-Platform ZBar port <br />
Uses CMake as the build system <br />
<br />
I have removed/moved files that don’t directly contribute to the locating and decoding of barcodes/qrcodes. They are located in the directory notused. If you would like their functionality .. then add them to the compilation yourself. :wink:

# Linux
mkdir build; <br />
cd build; <br />
cmake .. <br />
make -j$(nproc) install <br />

# Windows 10+ MSVC
Open VS 20xx Developer Command Prompt <br />
<br />
mkdir build; <br />
cd build; <br />
cmake -A x64 \ <br />
-DCMAKE_INSTALL_PREFIX=C:/lib/libzbar \ <br />
-DCMAKE_BUILD_TYPE=Release ..;
<br />
<br />
cmake --build . --config Release --target INSTALL <br />

# Android
## Linux:
./android.sh

## Windows 10+ MSVC
Open VS 20xx Developer Command Prompt. For the most part the same arguments can be used to compile against the Android NDK on Linux. You will need to adapt for your environment. Im using the Ninja generator provided with the NDK.<br />
<br />
cmake -G Ninja \ <br />
-DANDROID_ABI=x86 \ <br />
-DCMAKE_INSTALL_PREFIX=C:/lib/libzbar-android \ <br /> 
-DCMAKE_BUILD_TYPE=Release \ <br />
-DANDROID_NDK=C:\Users\brown\AppData\Local\Android\Sdk\ndk-bundle \ <br />
-DCMAKE_TOOLCHAIN_FILE=C:\Users\brown\AppData\Local\Android\Sdk\ndk-bundle\build\cmake\android.toolchain.cmake \ <br />
-DANDROID_NATIVE_API_LEVEL=android-26 \ <br />
-DBUILD_SHARED_LIBS=ON ..;
<br />
<br />
cmake --build . --config Release -- -j4 <br />
OR <br />
ninja build <br />
ninja install <br />

# macOS
mkdir build; <br />
cd build; <br />
cmake .. <br />
make -j$(sysctl -n hw.ncpu) install <br />

# iOS
You should have xcode and command line tools installed. Do it the "Apple" way and don't use homebrew. If you use homebrew the results are untested.

mkdir build; <br />
cd build; <br />
<br />
cmake -G Xcode \ <br />
-DCMAKE_INSTALL_PREFIX=~/lib/libzbar-ios \ <br />
-DCMAKE_BUILD_TYPE=Release \ <br />
-DIOS_ARCH=arm64 \ <br />
-DIPHONEOS_DEPLOYMENT_TARGET=13.2 \ <br />
-DCMAKE_TOOLCHAIN_FILE=../platforms/ios/cmake/Toolchains/Toolchain-iPhoneOS_Xcode.cmake ..;
<br />
<br />

## macOS Catalina or later
xcodebuild -configuration Release -scheme install -arch arm64 -sdk iphoneos <br />
[Catelina iOS build bug](https://github.com/sbrown89/libzbar/issues/4)

## macOS Mojave or earlier
xcodebuild -list -project libzbar.xcodeproj /* Optional to show info about the build*/ <br />
xcodebuild -configuration Release -scheme install <br />

## LEGAL: 
On Windows, Android, Apple, and iOS this will statically build and link the libiconv library.
For non-opensource / proprietary code, by dynamically linking to the final lib (libzbar), you will remain LGPL compliant. This, the original ZBar codebase and libiconv are released under the LGPL license.<br />
<br />
You should review the LGPL legal doc to ensure proper use.


## Original Sources
[zbar.sourceforge.net](http://zbar.sourceforge.net/)<br />
[libiconv](https://www.gnu.org/software/libiconv/)<br />
[iOS Toolchain](https://github.com/opencv/opencv)
