add_executable(node_style_test main.cpp)
target_link_libraries(node_style_test gnodegui Qt6::Core Qt6::Widgets
                      nlohmann_json::nlohmann_json)
