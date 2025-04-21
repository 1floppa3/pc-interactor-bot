add_library(CommandsLib
        CommandDispatcher.cpp
        HelpCommand.cpp
        StartCommand.cpp
        ShutdownCommand.cpp
        SystemInfoCommand.cpp
        MonitorCommand.cpp
        ScreenshotCommand.cpp
        CameraCommand.cpp
        LockCommand.cpp
        MediaToggleCommand.cpp
        MediaNextCommand.cpp
        MediaPrevCommand.cpp
        VolumeCommand.cpp
        RunCommand.cpp
        SayCommand.cpp
        SleepCommand.cpp
        HibernateCommand.cpp
        ProcessesCommand.cpp
        FindCommand.cpp
        KillCommand.cpp
)
target_include_directories(CommandsLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(CommandsLib
        PUBLIC CoreLib TelegramLib
)