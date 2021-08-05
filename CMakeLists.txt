cmake_minimum_required(VERSION 3.13)
project(PICo24_SDK C ASM)
set(CMAKE_C_STANDARD 99)

#set(XC16_DISABLE_DEFAULT_LINKER_SCRIPT 1)
#include(CMake/FindXC16.cmake)
#
#add_link_options(-Wl,--gc-sections,--no-select-objects,--heap=0,--stack=128)

function(add_library_whole NAME SRCS)
    # xc16-ld won't copy the interrupt handlers when linking with no extra options
    # Microchip's customer support can't give you anything other than "Warm Regards" lol

    add_library(${NAME}_ ${SRCS})
    add_library(${NAME} INTERFACE)
    add_dependencies(${NAME} ${NAME}_)
    target_link_libraries(${NAME} INTERFACE ${NAME}_)
    set_property(TARGET ${NAME} PROPERTY INTERFACE_LINK_LIBRARIES
            -Wl,--whole-archive,$<TARGET_FILE:${NAME}_>,--no-whole-archive)
endfunction()

function(pico24_add_board BOARD_NAME CPU_NAME)
#    message("-- PICo24: Adding board: ${BOARD_NAME} ${CPU_NAME}")
    set(BOARD_DIR "Boards/${BOARD_NAME}")
    set(OUT_NAME "PICo24_Board_${BOARD_NAME}")
    set(PICo24_Board_CPU_${BOARD_NAME} ${CPU_NAME})
    set(PICo24_Board_CPU_${BOARD_NAME} ${PICo24_Board_CPU_${BOARD_NAME}} PARENT_SCOPE)

    file(GLOB_RECURSE BOARD_SOURCES "${BOARD_DIR}/*.*")
    set(PICo24_Board_Sources_${BOARD_NAME} ${BOARD_SOURCES} PARENT_SCOPE)
    add_library_whole(${OUT_NAME} "${BOARD_SOURCES}")

    target_include_directories(${OUT_NAME} INTERFACE ${BOARD_DIR})
    target_compile_options(${OUT_NAME}_ PUBLIC -mcpu=${CPU_NAME})
    target_link_options(${OUT_NAME} INTERFACE -Wl,--script=${CMAKE_SOURCE_DIR}/${BOARD_DIR}/USBApp.ld)
endfunction()

function(pico24_enable_peripherals)
    foreach(NAME ${ARGV})
        message("-- PICo24: Enabling peripheral: ${NAME}")
        add_compile_definitions(PICo24_Enable_Peripheral_${NAME})
        endforeach()
endfunction()

function(pico24_enable_freertos)
    add_definitions(-DPICo24_FreeRTOS_Enabled=1)
    target_link_libraries(PICo24_Core PUBLIC PICo24_FreeRTOS)
    target_link_libraries(PICo24_ScratchLibc PUBLIC PICo24_FreeRTOS)
    target_link_libraries(lwipcore PUBLIC PICo24_FreeRTOS)
    target_link_libraries(lwipallapps PUBLIC PICo24_FreeRTOS)
endfunction()

function(pico24_use_board BOARD)
    message("-- PICo24: Using board: ${BOARD}, CPU: ${PICo24_Board_CPU_${BOARD}}")
    include_directories("Boards/${BOARD}")
    target_compile_options(PICo24_Core PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    target_compile_options(PICo24_ScratchLibc PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    target_compile_options(PICo24_FreeRTOS PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    target_compile_options(PICo24_Lua PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    target_compile_options(lwipcore PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    target_compile_options(lwipallapps PRIVATE -mcpu=${PICo24_Board_CPU_${BOARD}})
    add_compile_options(-mcpu="${PICo24_Board_CPU_${BOARD}}")
endfunction()

# Boards
pico24_add_board(PotatoPi_PICo24 24FJ256GB206)
pico24_add_board(CartBoy_RW_v2 24FJ256GB108)

# Global CFLAGS
include_directories(Components)

file(GLOB_RECURSE PICo24_ScratchLibc_SOURCES "Components/ScratchLibc/*.*")
add_library(PICo24_ScratchLibc "${PICo24_ScratchLibc_SOURCES}")

file(GLOB_RECURSE PICo24_FreeRTOS_SOURCES "Components/FreeRTOS/*.*")
add_library(PICo24_FreeRTOS "${PICo24_FreeRTOS_SOURCES}")

file(GLOB_RECURSE PICo24_Lua_SOURCES "Components/Lua/*.*")
add_library(PICo24_Lua "${PICo24_Lua_SOURCES}")

file(GLOB_RECURSE PICo24_Core_SOURCES "Components/PICo24/*.*")
add_library(PICo24_Core "${PICo24_Core_SOURCES}")
target_link_libraries(PICo24_Core PUBLIC PICo24_ScratchLibc)

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Components/lwip)
include(${CMAKE_CURRENT_SOURCE_DIR}/Components/lwip/src/Filelists.cmake)
target_link_libraries(lwipallapps PUBLIC lwipcore)

