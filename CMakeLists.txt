cmake_minimum_required(VERSION 3.25)
project(Lumy VERSION 0.1.1 LANGUAGES CXX)

# ===== Build type padrão =====
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ===== Fontes do exemplo =====
set(HELLO_TOWN_SOURCES
  src/main.cpp
  src/delta_time.cpp
  src/scene.cpp
  src/scene_stack.cpp
  src/boot_scene.cpp
  src/title_scene.cpp
  src/map_scene.cpp
  src/map.cpp
  src/texture_manager.cpp
  src/event_system.cpp
  src/save_system.cpp
)

add_executable(hello-town ${HELLO_TOWN_SOURCES})
target_compile_features(hello-town PRIVATE cxx_std_20)

# ===== Warnings =====
if(MSVC)
  target_compile_options(hello-town PRIVATE /W4 /permissive- /EHsc)
  target_compile_definitions(hello-town PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(hello-town PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ===== Dependências via vcpkg =====
# Dica: configure com o preset/arg:
# -DCMAKE_TOOLCHAIN_FILE="C:/Users/fanta/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows

# SFML 3: componentes com Maiúscula e targets SFML::*
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window Audio System)
target_link_libraries(hello-town PRIVATE
  SFML::Graphics SFML::Window SFML::Audio SFML::System
)

# JSON (alvo moderno)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(hello-town PRIVATE nlohmann_json::nlohmann_json)

# Lua (usa FindLua do CMake: fornece variáveis, não alvo)
find_package(Lua REQUIRED)
target_include_directories(hello-town PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(hello-town PRIVATE ${LUA_LIBRARIES})

# sol2 (header-only; vcpkg fornece alvo)
find_package(sol2 CONFIG REQUIRED)
target_link_libraries(hello-town PRIVATE sol2::sol2)

# tmxlite (via pkg-config no vcpkg)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TMXLITE REQUIRED IMPORTED_TARGET tmxlite)
target_link_libraries(hello-town PRIVATE PkgConfig::TMXLITE)

# ===== Includes próprios =====
target_include_directories(hello-town PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

# ===== Editor Lumy (wxWidgets) =====
set(EDITOR_SOURCES
    editor/main.cpp
    editor/editor_frame.cpp
    editor/project_tree_panel.cpp
    editor/property_grid_panel.cpp
    editor/viewport_panel.cpp
    editor/tileset_panel.cpp
    editor/map.cpp
    editor/map_manager.cpp
    editor/file_watcher.cpp
    editor/editor_events.cpp
    editor/project_manager.cpp
    editor/new_project_dialog.cpp
)

# wxWidgets
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base aui propgrid gl)

add_executable(lumy-editor ${EDITOR_SOURCES})
target_compile_features(lumy-editor PRIVATE cxx_std_20)

# wxWidgets setup
target_link_libraries(lumy-editor PRIVATE
  wx::core wx::base wx::aui wx::propgrid wx::gl
  nlohmann_json::nlohmann_json
)

# Definir flags específicas do wxWidgets para Windows
if(WIN32)
  target_compile_definitions(lumy-editor PRIVATE
    WXUSINGDLL
    wxMSVC_VERSION_AUTO
    _CRT_SECURE_NO_WARNINGS
  )
  # Configurar como aplicação Windows (não console)
  set_target_properties(lumy-editor PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
endif()

# Incluir diretórios
target_include_directories(lumy-editor PRIVATE
  ${CMAKE_SOURCE_DIR}/editor
)

# Warnings para o editor
if(MSVC)
  target_compile_options(lumy-editor PRIVATE /W4 /permissive- /EHsc /utf-8)
else()
  target_compile_options(lumy-editor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Saída do editor
set_property(TARGET lumy-editor PROPERTY RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")

# ===== Saída + cópia de assets do diretório game =====
set(OUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set_property(TARGET hello-town PROPERTY RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/game")
if(EXISTS "${ASSETS_DIR}")
  add_custom_command(TARGET hello-town POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copiando assets de ${ASSETS_DIR} para ${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E rm -rf   "${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${OUT_DIR}/game"
    VERBATIM
  )
endif()

# ===== Tests =====
enable_testing()
find_package(GTest CONFIG REQUIRED)
add_executable(lumy-tests
  tests/basic_startup.cpp
  tests/delta_time.cpp
  tests/scene_stack.cpp
  tests/scene_flow.cpp
  tests/title_scene.cpp
  tests/scene_loop_close.cpp
  tests/collision.cpp
  src/delta_time.cpp
  src/scene.cpp
  src/scene_stack.cpp
  src/boot_scene.cpp
  src/title_scene.cpp
  src/map_scene.cpp
  src/map.cpp
  src/texture_manager.cpp
  src/event_system.cpp
  src/save_system.cpp
)
target_link_libraries(lumy-tests PRIVATE
  GTest::gtest_main
  SFML::Graphics SFML::Window SFML::Audio SFML::System
  nlohmann_json::nlohmann_json
  sol2::sol2
  ${LUA_LIBRARIES}
  PkgConfig::TMXLITE
)
target_include_directories(lumy-tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${LUA_INCLUDE_DIR}
)
set_property(TARGET lumy-tests PROPERTY RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")
if(EXISTS "${ASSETS_DIR}")
  add_custom_command(TARGET lumy-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copiando assets de ${ASSETS_DIR} para ${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E rm -rf   "${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}/game"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${OUT_DIR}/game"
    VERBATIM
  )
endif()
add_custom_command(TARGET lumy-tests PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
)
add_test(NAME basic_startup COMMAND lumy-tests WORKING_DIRECTORY "${OUT_DIR}")

# ===== Instalação opcional =====
include(GNUInstallDirs)
install(TARGETS hello-town RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(EXISTS "${EXAMPLES_DIR}")
  install(DIRECTORY "${EXAMPLES_DIR}/" DESTINATION ${CMAKE_INSTALL_DATADIR}/lumy/hello-town)
endif()
