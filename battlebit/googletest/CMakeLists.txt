# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../src)

# 'Google_Tests_run' is the target name
add_executable(battleBit_tests char_buff.cpp game.cpp repl.cpp)
target_link_libraries(battleBit_tests gtest gtest_main battleBit_lib)

add_executable(homework_2 homework_2.cpp)
target_link_libraries(homework_2 gtest gtest_main battleBit_lib)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set_source_files_properties(homework_3.asm PROPERTIES LANGUAGE ASM_NASM)
add_executable(homework_3 homework_3.cpp homework_3.asm)
target_link_libraries(homework_3 gtest gtest_main)
