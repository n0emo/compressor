cmake_minimum_required(VERSION 3.14)
include(FetchContent)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(compressor)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    clap
    GIT_REPOSITORY "https://github.com/free-audio/clap"
    GIT_TAG "1.2.7"
)

FetchContent_MakeAvailable(clap)

find_package(Threads REQUIRED)

add_library(
    compressor SHARED
    ./src/compressor.c
    ./src/params.c
    ./src/plugin.c
    ./src/util.c
)

set_target_properties(compressor
    PROPERTIES
        C_STANDARD 23
        PREFIX ""
        SUFFIX ""
)

target_compile_options(
    compressor
    PRIVATE -g -Wall -Wextra -fPIC
)

target_include_directories(
    compressor
    PRIVATE ./include
)

target_link_libraries(
    compressor
    PUBLIC clap Threads::Threads
)

if(APPLE)
    set(INFO_PLIST_CONTENT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>compressor</string>
    <key>CFBundleIdentifier</key>
    <string>com.yourcompany.compressor</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>compressor</string>
    <key>CFBundlePackageType</key>
    <string>BNDL</string>
    <key>CFBundleShortVersionString</key>
    <string>${PROJECT_VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${PROJECT_VERSION}</string>
    <key>CLAPPluginFactory</key>
    <string>compressor_factory</string>
</dict>
</plist>")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" "${INFO_PLIST_CONTENT}")

    set_target_properties(compressor
        PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION "clap"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    )

    set(CLAP_BUNDLE_NAME "compressor.clap")
    set(CLAP_BUNDLE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CLAP_BUNDLE_NAME}")
    set(CLAP_CONTENTS_PATH "${CLAP_BUNDLE_PATH}/Contents")
    set(CLAP_MACOS_PATH "${CLAP_CONTENTS_PATH}/MacOS")

    add_custom_command(
        TARGET compressor
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CLAP_MACOS_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:compressor>"
            "${CLAP_MACOS_PATH}/compressor"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
            "${CLAP_CONTENTS_PATH}/Info.plist"
        COMMAND ${CMAKE_COMMAND} -E echo "APPL????" > "${CLAP_CONTENTS_PATH}/PkgInfo"
        COMMENT "Creating CLAP bundle for macOS"
    )

    add_custom_command(
        TARGET compressor
        POST_BUILD
        COMMAND codesign --force --deep --sign - "${CLAP_BUNDLE_PATH}"
        COMMENT "Ad-hoc code signing CLAP bundle"
    )

elseif(UNIX AND NOT APPLE)
    set_target_properties(compressor
        PROPERTIES
            SUFFIX ".clap"
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(
            TARGET compressor
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Stripping symbols for Linux build"
            COMMAND strip --strip-unneeded "$<TARGET_FILE:compressor>"
            COMMENT "Stripping debug symbols"
        )
    endif()

else()
    message(WARNING "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
