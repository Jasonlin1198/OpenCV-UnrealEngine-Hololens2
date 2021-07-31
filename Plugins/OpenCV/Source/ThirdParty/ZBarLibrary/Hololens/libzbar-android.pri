contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS += \
        $$PWD/arm32/libzbar.so

    LIBS += \
        $$PWD/arm32/libzbar.so
        
    INCLUDEPATH += \
        $$PWD/arm32/include
        
    DEPENDPATH += \
        $$PWD/arm32/include
}

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_EXTRA_LIBS += \
        $$PWD/arm64/libzbar.so

    LIBS += \
        $$PWD/arm64/libzbar.so
        
    INCLUDEPATH += \
        $$PWD/arm64/include
        
    DEPENDPATH += \
        $$PWD/arm64/include
}

contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_EXTRA_LIBS += \
        $$PWD/x86/libzbar.so

    LIBS += \
        $$PWD/x86/libzbar.so
        
    INCLUDEPATH += \
        $$PWD/x86/include
        
    DEPENDPATH += \
        $$PWD/x86/include
}

contains(ANDROID_TARGET_ARCH,x86_64) {
    ANDROID_EXTRA_LIBS += \
        $$PWD/x86_64/libzbar.so

    LIBS += \
        $$PWD/x86_64/libzbar.so
        
    INCLUDEPATH += \
        $$PWD/x86_64/include
        
    DEPENDPATH += \
        $$PWD/x86_64/include
}
