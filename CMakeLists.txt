cmake_minimum_required(VERSION 3.25)

project(JevWrapperPP VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_12_1
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(curl nlohmann_json)

add_library(JevWrapperPP src/TypeSafe_Jev/ts_jev_client.cpp)
add_library(JevWrapperPP::JevWrapperPP ALIAS JevWrapperPP)

target_compile_features(JevWrapperPP PUBLIC cxx_std_23)
target_link_libraries(JevWrapperPP PRIVATE libcurl nlohmann_json::nlohmann_json)

target_include_directories(JevWrapperPP
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
