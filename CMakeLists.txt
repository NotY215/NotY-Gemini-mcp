cmake_minimum_required(VERSION 3.20)
project(NotYGeminiMCP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

# Find Java
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Find Boost via vcpkg
find_package(Boost CONFIG REQUIRED COMPONENTS
        system
        filesystem
        json
        asio
        program_options
        property_tree
        beast
)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# Source files
set(SOURCES
        src/cpp/main.cpp
        src/cpp/web_server.cpp
        src/cpp/gemini_service.cpp
        src/cpp/vscode_manager.cpp
        src/cpp/encryption.cpp
        src/cpp/config_manager.cpp
        src/cpp/logger.cpp
        src/cpp/jni_bridge.cpp
)

set(HEADERS
        src/cpp/web_server.h
        src/cpp/gemini_service.h
        src/cpp/vscode_manager.h
        src/cpp/encryption.h
        src/cpp/config_manager.h
        src/cpp/logger.h
        src/cpp/jni_bridge.h
)

# Shared library
add_library(geminicore SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(geminicore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
        ${JNI_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(geminicore PRIVATE
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CURL_LIBRARIES}
        nlohmann_json::nlohmann_json
        fmt::fmt
        spdlog::spdlog
        ${JNI_LIBRARIES}
)

# Windows specific
if(WIN32)
    target_link_libraries(geminicore PRIVATE
            ws2_32
            winhttp
            crypt32
            shell32
            user32
            advapi32
            shlwapi
            version         # <-- ADD THIS for GetFileVersionInfo
    )
    target_compile_definitions(geminicore PRIVATE
            WIN32
            _WIN32
            _WINDOWS
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
            _WIN32_WINNT=0x0A00
    )
endif()