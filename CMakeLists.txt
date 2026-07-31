cmake_minimum_required(VERSION 3.20)
project(NotYGeminiMCP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Windows specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /W4 /EHsc /DUNICODE /D_UNICODE")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# Dependencies
find_package(Boost REQUIRED COMPONENTS 
    system 
    filesystem 
    json 
    asio 
    beast 
    program_options
    property_tree
)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(duktape REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/app_window.cpp
    src/web_server.cpp
    src/gemini_service.cpp
    src/vscode_manager.cpp
    src/encryption.cpp
    src/tray_icon.cpp
    src/config_manager.cpp
    src/logger.cpp
)

set(HEADERS
    src/app_window.h
    src/web_server.h
    src/gemini_service.h
    src/vscode_manager.h
    src/encryption.h
    src/tray_icon.h
    src/config_manager.h
    src/logger.h
)

# Executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    duktape::duktape
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc
)

# Windows resources
if(WIN32)
    set(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc)
    target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES})
endif()

# Copy web files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/web
    ${CMAKE_CURRENT_BINARY_DIR}/web
)