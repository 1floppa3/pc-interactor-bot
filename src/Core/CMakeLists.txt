add_library(CoreLib
        Config.cpp Logger.cpp
)
target_include_directories(CoreLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(CoreLib
        PUBLIC nlohmann_json::nlohmann_json
)