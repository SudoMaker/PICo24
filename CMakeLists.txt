cmake_minimum_required(VERSION 3.13)
project(PICo24_SDK C ASM)
set(CMAKE_C_STANDARD 99)

#add_link_options(-Wl,--gc-sections,--no-select-objects,--heap=0,--stack=128)

set(PICo24_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

message("-- PICo24 Source Dir: ${PICo24_SRC_DIR}")

include_directories(${PICo24_SRC_DIR}/Components)

file(GLOB_RECURSE PICo24_Core_SOURCES "${PICo24_SRC_DIR}/Components/PICo24/*.*")
set(PICo24_Core_SOURCES ${PICo24_Core_SOURCES} CACHE INTERNAL "")

file(GLOB_RECURSE PICo24_ScratchLibc_SOURCES "${PICo24_SRC_DIR}/Components/ScratchLibc/*.*")
set(PICo24_ScratchLibc_SOURCES ${PICo24_ScratchLibc_SOURCES} CACHE INTERNAL "")

file(GLOB_RECURSE PICo24_FreeRTOS_SOURCES "${PICo24_SRC_DIR}/Components/FreeRTOS/*.*")
set(PICo24_FreeRTOS_SOURCES ${PICo24_FreeRTOS_SOURCES} CACHE INTERNAL "")

file(GLOB_RECURSE PICo24_Lua_SOURCES "${PICo24_SRC_DIR}/Components/Lua/*.*")
set(PICo24_Lua_SOURCES ${PICo24_Lua_SOURCES} CACHE INTERNAL "")

set(LWIP_DIR ${PICo24_SRC_DIR}/Components/lwip)
set(LWIP_DIR ${LWIP_DIR} CACHE INTERNAL "")
include(${PICo24_SRC_DIR}/Components/lwip/src/Filelists.cmake)
set(lwipnoapps_SRCS ${lwipnoapps_SRCS} CACHE INTERNAL "")
set(lwipallapps_SRCS ${lwipallapps_SRCS} CACHE INTERNAL "")

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

function(pico24_target_configure TARGET BOARD PERIPH_LIST FEATURE_LIST)
    message("-- PICo24: Configuring for target: ${TARGET}")

    set(BOARD_DIR ${PICo24_SRC_DIR}/Boards/${BOARD})
    message("-- PICo24: Board directory: ${BOARD_DIR}")

    file(READ ${BOARD_DIR}/CPU.txt CPU_NAME)
    message("-- PICo24: Using board: ${BOARD}, CPU: ${CPU_NAME}")

    set(BOARD_OUT_NAME "PICo24_Board_${BOARD}_For_${TARGET}")
    file(GLOB_RECURSE BOARD_SOURCES "${BOARD_DIR}/*.*")
    set(PICo24_Board_Sources_${BOARD_NAME} ${BOARD_SOURCES} PARENT_SCOPE)
    add_library(${BOARD_OUT_NAME} "${BOARD_SOURCES}")

    target_include_directories(${BOARD_OUT_NAME} PUBLIC ${BOARD_DIR} ${PICo24_SRC_DIR}/Components)
    target_compile_options(${BOARD_OUT_NAME} PUBLIC -mcpu=${CPU_NAME})
    target_link_options(${BOARD_OUT_NAME} INTERFACE -Wl,--script=${BOARD_DIR}/USBApp.ld)

    add_library(PICo24_Core_For_${TARGET} "${PICo24_Core_SOURCES}")
    target_include_directories(PICo24_Core_For_${TARGET} PUBLIC ${BOARD_DIR} ${PICo24_SRC_DIR}/Components)
    target_compile_options(PICo24_Core_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})


    foreach(PERIPH ${PERIPH_LIST})
        message("-- PICo24: Enabling peripheral: ${PERIPH}")
        target_compile_definitions(PICo24_Core_For_${TARGET} PUBLIC PICo24_Enable_Peripheral_${PERIPH})
        target_compile_definitions(${BOARD_OUT_NAME} PUBLIC PICo24_Enable_Peripheral_${PERIPH})
    endforeach()

    add_library(PICo24_ScratchLibc_For_${TARGET} "${PICo24_ScratchLibc_SOURCES}")
    target_compile_options(PICo24_ScratchLibc_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})
    target_include_directories(PICo24_ScratchLibc_For_${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components)

    add_library(PICo24_FreeRTOS_For_${TARGET} "${PICo24_FreeRTOS_SOURCES}")
    target_compile_options(PICo24_FreeRTOS_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})
    target_include_directories(PICo24_FreeRTOS_For_${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components)

    add_library(PICo24_Lua_For_${TARGET} "${PICo24_Lua_SOURCES}")
    target_compile_options(PICo24_Lua_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})
    target_include_directories(PICo24_Lua_For_${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components)

    foreach(FEATURE ${FEATURE_LIST})
        if (${FEATURE} STREQUAL "FreeRTOS")
            message("-- PICo24: Enabling feature: FreeRTOS")
            target_compile_definitions(PICo24_Core_For_${TARGET} PUBLIC -DPICo24_FreeRTOS_Enabled=1)
            target_compile_definitions(PICo24_ScratchLibc_For_${TARGET} PUBLIC -DPICo24_FreeRTOS_Enabled=1)
            target_compile_definitions(PICo24_FreeRTOS_For_${TARGET} PUBLIC -DPICo24_FreeRTOS_Enabled=1)
            target_compile_definitions(${BOARD_OUT_NAME} PUBLIC -DPICo24_FreeRTOS_Enabled=1)
        endif()
    endforeach()

    add_library(lwipcore_For_${TARGET} ${lwipnoapps_SRCS})
    target_compile_options(lwipcore_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})
    target_include_directories(lwipcore_For_${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components ${LWIP_DIR}/src/include)

    add_library(lwipallapps_For_${TARGET} ${lwipallapps_SRCS})
    target_compile_options(lwipallapps_For_${TARGET} PRIVATE -mcpu=${CPU_NAME})
    target_include_directories(lwipallapps_For_${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components ${LWIP_DIR}/src/include)

    add_library(PICo24_For_${TARGET} INTERFACE)
    target_link_libraries(PICo24_For_${TARGET} INTERFACE "-Wl,--start-group" PICo24_Core_For_${TARGET} ${BOARD_OUT_NAME} PICo24_ScratchLibc_For_${TARGET} PICo24_FreeRTOS_For_${TARGET} PICo24_Lua_For_${TARGET} lwipcore_For_${TARGET} lwipallapps_For_${TARGET} "-Wl,--end-group")
    target_compile_options(PICo24_For_${TARGET} INTERFACE -mcpu=${CPU_NAME})

    target_link_libraries(${TARGET} PICo24_For_${TARGET})
    target_compile_options(${TARGET} PUBLIC -mcpu=${CPU_NAME})
    target_include_directories(${TARGET} PUBLIC ${PICo24_SRC_DIR}/Components)
    target_link_options(${TARGET} PUBLIC -mcpu=${CPU_NAME} -Wl,--script=${BOARD_DIR}/USBApp.ld -Wl,--gc-sections,--no-select-objects,--heap=0,--stack=128)

    message("-- PICo24: Done configuring for target: ${TARGET}")
endfunction()
