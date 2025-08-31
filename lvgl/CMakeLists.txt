#此文件从模板自动生成! 请勿更改!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 定义一个项目。虽然通常是编译为库，但定义项目有助于管理
project(lvgl LANGUAGES C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_JOB_POOLS "compile=${N}")
    set(CMAKE_JOB_POOL_COMPILE compile)
endif()


find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(CMAKE_DEPENDS_IN_PROJECT_ONLY TRUE)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

#Uncomment for hardware floating point
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif()


add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx)

# 设置 LVGL 相关的配置选项
# 选项：是否简单包含 lvgl.h (使用 #include "lvgl.h" 而不是相对路径)
option(LV_LVGL_H_INCLUDE_SPLE "Use #include \"lvgl.h\" instead of #include \"../../lvgl.h\"" ON)
# 选项：是否简单包含 lv_conf.h (使用 #include "lv_conf.h" 而不是相对路径)
option(LV_CONF_INCLUDE_SIMPLE "Use #include \"lv_conf.h\" instead of #include \"../../lv_conf.h\"" ON)

# 重要：如果你的 lv_conf.h 不在 lvgl 目录下，请设置其路径
# 例如，如果你的 lv_conf.h 在项目根目录，取消注释并修改下一行
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/lv_conf.h" CACHE STRING "Path to lv_conf.h")

include_directories(
    ../Core/Inc
    ../Drivers/STM32F4xx_HAL_Driver/Inc
    ../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ../Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ../Drivers/CMSIS/Include
    ../User/Inc
    .
    ../
    ../Gui/lvgl
    ../Gui/generated
    ../Gui/generated/guider_fonts
    ../Gui/generated/guider_customer_fonts
    ../Gui/custom
    ../lvgl/
    ../USB_DEVICE/App
    ../USB_DEVICE/Target
    ../Middlewares/Third_Party/FatFs/src
    ../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc
    ../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    ../FATFS/App
    ../FATFS/Target
    ../Utils/Inc
    ../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang
    ../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ../Middlewares/Third_Party/FreeRTOS/Source/include
    ../Middlewares/Third_Party/FreeRTOS/Source/portable
    ../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    F:/ProgramFiles/GNU Arm Embedded Toolchain/10 2021.10/arm-none-eabi/include/reent.h
)

# 递归收集 LVGL 源文件 (核心源文件)
file(GLOB_RECURSE LVGL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.S"  # 如果需要汇编文件
)

# 可选：收集移植文件 (如果你有自定义的移植层，例如显示、触摸屏、文件系统等)
# 假设你的移植文件在 lvgl/porting 目录，并且主项目已经包含了这个路径
file(GLOB_RECURSE LVGL_PORTING_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/porting/*.c"
)

# 合并源文件列表
set(ALL_LVGL_SOURCES ${LVGL_SOURCES} ${LVGL_PORTING_SOURCES})

# 创建 LVGL 静态库目标
add_library(lvgl STATIC ${ALL_LVGL_SOURCES})

# 为库目标设置别名，方便链接时使用 (现代 CMake 推荐做法)
add_library(lvgl::lvgl ALIAS lvgl)

# 设置库目标的编译定义 (预处理宏)
target_compile_definitions(lvgl
    PUBLIC
        # 根据选项设置包含方式宏
        $<$<BOOL:${LV_LVGL_H_INCLUDE_SIMPLE}>:LV_LVGL_H_INCLUDE_SIMPLE>
        $<$<BOOL:${LV_CONF_INCLUDE_SIMPLE}>:LV_CONF_INCLUDE_SIMPLE>
        # 如果你的 lv_conf.h 路径是通过 LV_CONF_PATH 设置的，添加以下宏
        # $<$<BOOL:${LV_CONF_PATH}>:LV_CONF_PATH=${LV_CONF_PATH}>
    PRIVATE
        # 可以在这里添加 LVGL 库内部需要的私有宏定义
)

# 设置库目标的包含目录
target_include_directories(lvgl
    PUBLIC  # PUBLIC 表示链接此库的目标也会继承这些包含目录
        ${CMAKE_CURRENT_SOURCE_DIR}   # 用于包含 lvgl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src  # 核心头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/porting  # 移植层头文件 (如果有)
        # 注意：lv_conf.h 所在目录应该已经在主项目的 include_directories 中
)
