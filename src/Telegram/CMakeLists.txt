add_library(TelegramLib
        TelegramApi.cpp
)
target_include_directories(TelegramLib
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(TelegramLib
        PUBLIC cpr::cpr nlohmann_json::nlohmann_json
)