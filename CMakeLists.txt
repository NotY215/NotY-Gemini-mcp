cmake_minimum_required(VERSION 3.20)
project(NotYGeminiMCP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Find Java
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Find Boost
find_package(Boost REQUIRED COMPONENTS 
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

# Generate JNI headers
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/com_noty_geminimcp_NativeBridge.h
    COMMAND ${Java_JAVAC_EXECUTABLE} -h ${CMAKE_CURRENT_BINARY_DIR} 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/java/com/noty/geminimcp/NativeBridge.java
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/java/com/noty/geminimcp/NativeBridge.java
    COMMENT "Generating JNI headers"
)

# Shared library
add_library(geminicore SHARED ${SOURCES})

# Include directories
target_include_directories(geminicore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${JNI_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries
target_link_libraries(geminicore PRIVATE
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    fmt::fmt
    spdlog::spdlog
    ws2_32
    winhttp
    crypt32
    shell32
    user32
    gdi32
    advapi32
    shlwapi
)

# Set output directory
set_target_properties(geminicore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)