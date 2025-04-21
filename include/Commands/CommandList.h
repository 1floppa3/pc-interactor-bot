#pragma once

#include <Commands/CameraCommand.h>
#include <Telegram/TelegramApi.h>
#include <Commands/HelpCommand.h>
#include <Commands/HibernateCommand.h>
#include <Commands/LockCommand.h>
#include <Commands/MediaNextCommand.h>
#include <Commands/MediaPrevCommand.h>
#include <Commands/MediaToggleCommand.h>
#include <Commands/MonitorCommand.h>
#include <Commands/ProcessesCommand.h>
#include <Commands/RunCommand.h>
#include <Commands/SayCommand.h>
#include <Commands/ScreenshotCommand.h>
#include <Commands/ShutdownCommand.h>
#include <Commands/SleepCommand.h>
#include <Commands/StartCommand.h>
#include <Commands/SystemInfoCommand.h>
#include <Commands/VolumeCommand.h>
#include <Commands/FindCommand.h>
#include <Commands/KillCommand.h>

#define COMMAND_LIST(X)               \
    X(KillCommand)                    \
    X(FindCommand)                    \
    X(CameraCommand)                  \
    X(SystemInfoCommand)              \
    X(HibernateCommand)               \
    X(LockCommand)                    \
    X(MediaNextCommand)               \
    X(MediaPrevCommand)               \
    X(MediaToggleCommand)             \
    X(MonitorCommand)                 \
    X(ProcessesCommand)               \
    X(RunCommand)                     \
    X(SayCommand)                     \
    X(ScreenshotCommand)              \
    X(ShutdownCommand)                \
    X(SleepCommand)                   \
    X(VolumeCommand)                  \
    X(HelpCommand)                    \
    X(StartCommand)