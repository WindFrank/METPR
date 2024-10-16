aux_source_directory(. DIR_GENERATOR_SOURCE)

add_library(LIB_GENERATOR ${DIR_GENERATOR_SOURCE})

target_link_libraries(LIB_GENERATOR Boost::system)