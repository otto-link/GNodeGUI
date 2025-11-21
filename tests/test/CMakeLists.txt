add_executable(test main.cpp)
target_link_libraries(test gnodegui Qt6::Core Qt6::Widgets nlohmann_json::nlohmann_json)
set_property(TARGET test PROPERTY AUTOMOC ON) # to enable automoc on main.cpp