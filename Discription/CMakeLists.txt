aux_source_directory(. DIR_DISCRIPTION_SOURCE)

add_library(LIB_DISCRIPTION ${DIR_DISCRIPTION_SOURCE})

target_link_libraries(LIB_DISCRIPTION Boost::system)