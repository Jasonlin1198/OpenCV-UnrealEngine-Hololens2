#!/bin/bash
#
# @author:  Stephen Brown
# @date:    8/22/2019

if [[ -z $ANDROID_NDK_HOME && -z $1 ]]; then
    echo "ANDROID_NDK_HOME environment variable must be set or passed in as arg. Ex: ~/Android/Sdk/ndk/20.*.*";
    exit 1;
fi

if [[ ! -z $1 ]]; then
    export ANDROID_NDK_HOME=$1
fi

# Default values, change for your needs, will cli args at some point
ANDROID_API=24;
ANDROID_ABI="";
CMAKE_INSTALL_PREFIX="";
NINJA_BIN="";

# Ensure cmake is >= 3
if [[ -z $(cmake --version | grep -P "3\.[0-9]+\.[0-9]+") ]]; then
    echo "cmake Major version must be >= 3";
    exit 1;
fi

# Find ninja
status=$?;
which ninja &> /dev/null;
status=$?;
if [[ $status -eq 0 ]]; then
    NINJA_BIN=$(which ninja);
else
    echo "Ninja build system does not exists, install to continue? y/n";
    read resp;
    
    if [[ $resp == "y" ]]; then
        # Try dnf, yum, apt in order
        for pkgman in "dnf" "yum" "apt"; do
            status=$?;
            which $pkgman &> /dev/null;
            status=$?;
            
            if [[ $status -ne 0 ]]; then
                continue;
            else
                sudo $pkgman install -y ninja-build;
                break;
            fi
        done
        
        status=$?;
        which ninja &> /dev/null;
        status=$?;
        if [[ $status -eq 0 ]]; then
            NINJA_BIN=$(which ninja);
        else
            echo "Could not install ninja build system!";
            exit 1;
        fi
    else
        echo "Aborting!";
        exit 1;
    fi
fi

export PATH="$NINJA_BIN":"$PATH";

if [[ $CMAKE_INSTALL_PREFIX == "" ]]; then
    CMAKE_INSTALL_PREFIX="$(pwd)/libzbar-android";
fi

# Purge install location
status=$?;
rm -rf $CMAKE_INSTALL_PREFIX;
status=$?;

if [[ $status -ne 0 ]]; then
    if [[ -d $CMAKE_INSTALL_PREFIX ]]; then
        echo "Requesting sudo access for purge of install location:"
        status=$?;
        sudo rm -rf $CMAKE_INSTALL_PREFIX;
        status=$?;
        
        if [[ $status -ne 0 ]]; then
            echo "Error: Not able to purge install location!";
            exit 1;
        fi
    fi
fi

mkdir build &> /dev/null;
cd build;

for arch in "x86_64" "x86" "arm64" "arm32"; do
    status=$?;
    mkdir -p "$CMAKE_INSTALL_PREFIX/$arch";
    status=$?;
    
    if [[ $status -ne 0 ]]; then
        echo "Requesting sudo access for creation of install location: $arch"
        status=$?;
        sudo mkdir -p "$CMAKE_INSTALL_PREFIX/$arch";
        status=$?;
        
        if [[ $status -ne 0 ]]; then
            echo "Error: Not able to create install location!";
            exit 1;
        fi
    fi
    
    # If needed set api level by arch
    case $arch in
        x86)
            ANDROID_API=24;
            ANDROID_ABI="x86";
            ;;
        arm64)
            ANDROID_API=24;
            ANDROID_ABI="arm64-v8a";
            ;;
        arm32)
            ANDROID_API=24;
            ANDROID_ABI="armeabi-v7a";
            ;;
        x86_64)
            ANDROID_API=24;
            ANDROID_ABI="x86_64";
            ;;
    esac
    
    echo -e "\tBuilding for $ANDROID_ABI";
    
    status=$?;
    cmake -G Ninja \
    "-DANDROID_ABI=$ANDROID_ABI"\
    "-DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX/$arch"\
    "-DCMAKE_BUILD_TYPE=Release" "-DANDROID_NDK=$ANDROID_NDK_HOME"\
    "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME\build\cmake\android.toolchain.cmake"\
    "-DANDROID_NATIVE_API_LEVEL=android-$ANDROID_API"\
    "-DBUILD_SHARED_LIBS=ON"\
    .. &> /dev/null;
    status=$?;
    
    if [[ $status -eq 0 ]]; then
        cmake --build . --config Release -- -j$(nproc);
        
        status=$?;
        $NINJA_BIN install &> /dev/null;
        status=$?;
        
        if [[ $status -ne 0 ]]; then
            if [[ -d $CMAKE_INSTALL_PREFIX/$arch ]]; then
                echo "Requesting sudo access for install:"
                status=$?;
                sudo $NINJA_BIN install &> /dev/null;
                status=$?;
                
                if [[ $status -ne 0 ]]; then
                    echo "Error: Not able to install!";
                    exit 1;
                else
                    echo "Installed $arch";
                fi
            fi
        else 
            echo -e "\tInstalled $arch";
        fi
        
        rm -rf ./*
    else
        echo "Please fix CMake Errors!";
        exit 1;
    fi
done
cd ..;
rm -rf build;

status=$?;
cp ./libzbar-android.pri $CMAKE_INSTALL_PREFIX
status=$?;

if [[ $status -ne 0 ]]; then
    if [[ -d $CMAKE_INSTALL_PREFIX ]]; then
        echo "Requesting sudo access for copying of pri file:"
        status=$?;
        sudo cp ./libzbar-android.pri $CMAKE_INSTALL_PREFIX
        status=$?;
        
        if [[ $status -ne 0 ]]; then
            echo "Error: Not able to copy pri file!";
            exit 1;
        fi
    fi
fi

exit 0;
