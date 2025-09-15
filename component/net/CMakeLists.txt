project(http)

aux_source_directory(./ SOURCE)

include_directories(http
    PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/component/linux_component
)

add_library(http STATIC ${SOURCE})

target_link_libraries(http 
                cjson
                curl
                ssl
                crypto
                nghttp2
                dl
                linux_component
                )     