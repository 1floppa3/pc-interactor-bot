add_library(CommandsLib
        CommandRegistry.cpp
        HelpCommand.cpp
        StartCommand.cpp
        ShutdownPCCommand.cpp
)
target_include_directories(CommandsLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(CommandsLib
        PUBLIC
        CoreLib
        TelegramLib
)