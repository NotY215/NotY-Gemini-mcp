cmake_minimum_required(VERSION 3.20)
project(NotYGeminiMCP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Windows specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Quick QuickControls2 QuickTemplates2)

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

# QML resources
set(QML_FILES
    ui/main.qml
    ui/VSCodeSetup.qml
    ui/ApiKeySetup.qml
    ui/ServerControl.qml
    ui/TermsDialog.qml
    ui/ToastNotification.qml
    ui/styles/Theme.qml
)

# Qt resources
set(QRC_FILES
    resources.qrc
)

# Generate QML resources
qt5_add_resources(QML_RESOURCES ${QRC_FILES})

# Executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${QML_RESOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::QuickTemplates2
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
    comctl32
    ole32
    oleaut32
    advapi32
    shlwapi
)

# Add resources if available
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc)
endif()

# Copy QML files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_BINARY_DIR}/ui
    COMMENT "Copying QML files to build directory"
)

# Windows specific definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32
        _WIN32
        _WINDOWS
        NOMINMAX
    )
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)