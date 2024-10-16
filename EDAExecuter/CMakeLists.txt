aux_source_directory(. DIR_EDAEXECUTER_SOURCE)

add_library(LIB_EDAEXECUTER ${DIR_EDAEXECUTER_SOURCE})

target_link_libraries(LIB_EDAEXECUTER Boost::system)