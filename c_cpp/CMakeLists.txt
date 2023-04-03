cmake_minimum_required(VERSION 2.8)
project( test )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# 添加所有的cpp文件
FILE(GLOB SRC_FILE_CPP ${PROJECT_SOURCE_DIR}/*.cpp)

# 添加所有的c文件
FILE(GLOB SRC_FILE_C ${PROJECT_SOURCE_DIR}/*.c)

# 添加头文件路径
include_directories(
  ${PROJECT_SOURCE_DIR}/
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILE_CPP} ${SRC_FILE_C})

# add_executable( test test.c )

target_link_libraries( test ${OpenCV_LIBS} )
