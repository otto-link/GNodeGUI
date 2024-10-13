project(gnodegui)

# Qt6
set(CMAKE_AUTOMOC ON)

# --- sources GUI headers need to be added in add_executable, otherwise the moc
# won't parse them
file(GLOB_RECURSE GNODEGUI_GUI_INCLUDES
     ${CMAKE_CURRENT_SOURCE_DIR}/include/gnodegui/*.hpp)

file(GLOB_RECURSE GNODEGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${GNODEGUI_SOURCES} ${GNODEGUI_GUI_INCLUDES})

set(GNODEGUI_INCLUDE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE
                     ${CMAKE_CURRENT_SOURCE_DIR}/../external/json/include)

target_include_directories(${PROJECT_NAME} PUBLIC ${GNODEGUI_INCLUDE})

# Link libraries
target_link_libraries(
  ${PROJECT_NAME} PRIVATE spdlog::spdlog Qt6::Core Qt6::Widgets
                          nlohmann_json::nlohmann_json)
