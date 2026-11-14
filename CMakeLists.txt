cmake_minimum_required(VERSION 3.16)
project(akai_at_home CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # lets editors find Qt headers
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUIDSYNTH REQUIRED IMPORTED_TARGET fluidsynth)
pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)

set(LESSONS guide.cpp lessons_a.cpp lessons_b.cpp lessons_c.cpp lessons_d.cpp lessons_e.cpp)
add_executable(akai_at_home main.cpp song.cpp engine.cpp mainwindow.cpp panels.cpp topbar.cpp melody.cpp chordguide.cpp ${LESSONS})
target_link_libraries(akai_at_home Qt6::Widgets PkgConfig::FLUIDSYNTH PkgConfig::SNDFILE)

add_executable(song_test song_test.cpp song.cpp ${LESSONS})
target_link_libraries(song_test Qt6::Core)

add_executable(piano_pals kidspiano.cpp)
target_link_libraries(piano_pals Qt6::Widgets PkgConfig::FLUIDSYNTH)
