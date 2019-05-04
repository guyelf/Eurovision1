cmake_minimum_required(VERSION 3.10)
project(ex1_v3 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic-errors -Werror -DNDEBUG")

include_directories(.)
#include_directories(libmtm)
include_directories(tests)


add_executable(ex1_v3
        list.h
        tests/main.c
        #tests/eurovisionTests.c
        #tests/eurovisionTestsMain.c
        #testsNisanItamar/eurovisionTests.h
        #testsNisanItamar/eurovisionTests.c
        #testsNisanItamar/eurovisionTestsMain.c
        #testsfacebook/main.c
        #testsfacebook/eurovisionFunctionalityTests.c
        Helper.c
        Judge.c
        State.c
        eurovision.c
        Map.c
        )


target_link_libraries(ex1_v3 ${CMAKE_CURRENT_SOURCE_DIR}/libmtm/win32/libmtm.a)