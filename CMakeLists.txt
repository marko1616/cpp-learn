cmake_minimum_required(VERSION 3.20)
project(Wireana VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/cxxopts)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/json)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/libmaxminddb)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus)
include_directories(${CMAKE_SOURCE_DIR}/libs/cxxopts)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Common++/header)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Packet++/header)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Pcap++/header)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts json libmaxminddb Common++ Packet++ Pcap++)