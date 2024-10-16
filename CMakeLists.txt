cmake_minimum_required(VERSION 3.20.0)

project(Vgenerator VERSION 1.0.0)

# add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

include_directories(../)

add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

set(BUILD_SHARED_LIBS ON)

find_package(Boost REQUIRED COMPONENTS system filesystem serialization)

include_directories(${Boost_INCLUDE_DIRS})

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

include_directories(${Python3_INCLUDE_DIRS})

link_directories(${Python3_LIBRARIES})

# #python3-config --includes
# include_directories("/usr/include/python3.10")
# #python3-config --ldflags
# link_directories("/usr/lib/python3.10/config-3.10-x86_64-linux-gnu")

add_subdirectory(ActionMutateLead)

add_subdirectory(Discription)

add_subdirectory(EDAExecuter)

add_subdirectory(Generator)

# # add_subdirectory(HDLProgramSets)

add_subdirectory(lib)

add_subdirectory(MRVerify)

# add_subdirectory(Test)

aux_source_directory(. DIR_VGENERATOR_SOURCE)

add_executable(Vgenerator ${DIR_VGENERATOR_SOURCE})

target_link_libraries(Vgenerator 
    ${Boost_LIBRARIES}
    Python3::Python
    # -lpython3.10
    LIB_ACTIONMUTATELEAD
    LIB_DISCRIPTION
    LIB_EDAEXECUTER
    LIB_GENERATOR
    LIB_LIBJSON
    LIB_TINYXML
    # LIB_RAPIDXML_DOC_POSRPROCESSOR_SOURCE
    LIB_MRVERIFY
    # LIB_TEST
)
