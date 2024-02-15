cmake_minimum_required (VERSION 3.1)
project (ap_wizard)

if (MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
set(CMAKE_WIN32_EXECUTABLE true)
endif(MSVC)

find_package(wxWidgets CONFIG REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(
  vendor/nlohmann
  ${yaml-cpp_INCLUDE_DIRS}
)

add_executable(ap_wizard
  src/main.cc
  src/wizard_frame.cc
  src/game_definition.cc
  src/world_window.cc
  src/world.cc
  src/wizard_editor.cc
  src/yaml_editor.cc
  src/random_choice_dialog.cc
)
set_property(TARGET ap_wizard PROPERTY CXX_STANDARD 20)
set_property(TARGET ap_wizard PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(ap_wizard PRIVATE wx::core wx::base wx::stc yaml-cpp::yaml-cpp)
