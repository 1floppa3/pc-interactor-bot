add_library(CoreLib
        Config.cpp
        AppManager.cpp
        SystemController.cpp
)
target_include_directories(CoreLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(CoreLib
        PUBLIC nlohmann_json::nlohmann_json fmt::fmt
        ole32 oleaut32 wbemuuid mf shlwapi mfuuid mfreadwrite mfplat sapi powrprof
)