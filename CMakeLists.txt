cmake_minimum_required(VERSION 3.29)
project(pc_interactor_bot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# CPR
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static CPR" FORCE)
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "Use internal libcurl" FORCE)
FetchContent_Declare(cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG dd967cb48ea6bcbad9f1da5ada0db8ac0d532c06) # 1.11.2
FetchContent_MakeAvailable(cpr)

# JSON
FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

# FMT
FetchContent_Declare(fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         123913715afeb8a437e6388b4473fcc4753e1c9a) # 11.1.4
FetchContent_MakeAvailable(fmt)

# ICO
set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/resources/favicon.rc")
enable_language(RC)


add_subdirectory(src/Core)
add_subdirectory(src/Telegram)
add_subdirectory(src/Commands)
add_subdirectory(src/Bot)

add_executable(pc_interactor_bot "src/main.cpp")
target_link_libraries(pc_interactor_bot PRIVATE CoreLib TelegramLib CommandsLib BotLib)
target_sources(pc_interactor_bot PRIVATE ${APP_ICON_RESOURCE})