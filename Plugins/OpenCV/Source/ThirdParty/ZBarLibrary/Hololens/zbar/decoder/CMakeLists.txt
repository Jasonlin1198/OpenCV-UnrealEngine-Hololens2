cmake_minimum_required(VERSION 3.3)
set(SOURCES
    code128.c
    code39.c
    ean.c
    i25.c
    pdf417.c
    qr_finder.c
)

set(HEADERS
    ${ZBAR_MAIN}
    code128.h
    code39.h
    ean.h
    i25.h
    pdf417.h
    pdf417_hash.h
    qr_finder.h
)

add_library(${LDECODER} STATIC ${SOURCES} ${HEADERS})
if(WIN32)
    target_compile_definitions(${LDECODER} PUBLIC LIB_ZBAR_BUILD)
endif(WIN32)

target_link_libraries(${LDECODER} ${LCONFIG})
