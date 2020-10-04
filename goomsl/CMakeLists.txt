cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FLEX_TARGET(goomslScanner goomsl_lex.l ${CMAKE_CURRENT_BINARY_DIR}/goomsl_lex.c)
BISON_TARGET(goomslParser goomsl_yacc.y ${CMAKE_CURRENT_BINARY_DIR}/goomsl_yacc.c)

ADD_LIBRARY(goomsl STATIC
            goomsl.c
            goomsl_heap.c
            jitc_test.c
            jitc_x86.c
            goomsl_hash.cpp
            ${FLEX_goomslScanner_OUTPUTS}
            ${BISON_goomslParser_OUTPUTS}
           )

target_include_directories(goomsl PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR}
                          )
