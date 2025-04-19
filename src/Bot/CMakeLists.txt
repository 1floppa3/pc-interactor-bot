add_library(BotLib
        TelegramBot.cpp
)
target_include_directories(BotLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
