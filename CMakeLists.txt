#设置cmake版本号
cmake_minimum_required(VERSION 3.4.0)

#设置工程名称
set(PROJECT_NAME MIG-Monitor)
project(${PROJECT_NAME})

#设置Qt安装路径
set(CMAKE_PREFIX_PATH "C:/Qt/5.11.0/msvc2017_64") 

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON) 

#设置自动生成moc文件，一定要设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
SET(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11) 

set (BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PlatformName}")

#查找需要的Qt库文件，最好每一个库都要写，Qt也会根据依赖关系自动添加
find_package(Qt5 COMPONENTS Core Widgets Gui Charts REQUIRED)

set(OpenCV_DIR "D:/opencv347/opencv/build")
find_package(OpenCV REQUIRED)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp") 
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h") 
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui") 

FILE(GLOB LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.lib")

SET(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/rcc.qrc)

#通过Ui文件生成对应的头文件，一定要添加
QT5_WRAP_UI(WRAP_FILES ${UI_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

#添加资源文件
#set(RCC_FILES rcc.qrc)

#将ui文件和生成文件整理在一个文件夹中，非必须
source_group("Form Files" FILES ${UI_FILES})
#source_group("Generated Files" FILES ${WRAP_FILES})

#创建工程文件
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEAD_FILES} ${UI_FILES} ${WRAP_FILES} ${RESOURCES})

link_directories(${PROJECT_SOURCE_DIR}/lib/)

#添加Qt5依赖项
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Charts)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} ${LIB_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

SET(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})