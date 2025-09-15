project(linux_component)

aux_source_directory(./ SOURCE)

include_directories(linux_component PUBLIC
                        .
                        )

add_library(linux_component STATIC ${SOURCE})

