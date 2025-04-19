add_library(CommandsLib
        CommandRegistry.cpp
        HelpCommand.cpp
        StartCommand.cpp
)
target_include_directories(CommandsLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
