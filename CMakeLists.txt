cmake_minimum_required(VERSION 3.20)
project(Wireana VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

include(GNUInstallDirs)
include(CheckTypeSize)
check_type_size("unsigned __int128" UINT128)
check_type_size("unsigned int __attribute__((mode(TI)))" UINT128_USING_MODE)
if(HAVE_UINT128)
  set(MMDB_UINT128_USING_MODE 0)
  set(MMDB_UINT128_IS_BYTE_ARRAY 0)
elseif(HAVE_UINT128_USING_MODE)
  set(MMDB_UINT128_USING_MODE 1)
  set(MMDB_UINT128_IS_BYTE_ARRAY 0)
else()
  set(MMDB_UINT128_USING_MODE 0)
  set(MMDB_UINT128_IS_BYTE_ARRAY 1)
endif()

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/cxxopts)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/json)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/libmaxminddb)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus)
include_directories(${CMAKE_SOURCE_DIR}/libs/cxxopts)
include_directories(${CMAKE_SOURCE_DIR}/libs/json/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/libmaxminddb/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Common++/header)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Packet++/header)
include_directories(${CMAKE_SOURCE_DIR}/libs/PcapPlusPlus/Pcap++/header)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts json maxminddb Common++ Packet++ Pcap++)