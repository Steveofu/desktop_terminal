project(wifi)

aux_source_directory(./ SOURCE)
aux_source_directory(./linux_component SOURCE)
aux_source_directory(./utils SOURCE)

include_directories(wifi PUBLIC
                            .
                            ./linux_component
                            ./utils
                            ${CMAKE_SOURCE_DIR}/component/linux_component
                            )


add_library(wifi STATIC ${SOURCE})

link_directories(./libs)

target_link_libraries(wifi wpa_client linux_component)
