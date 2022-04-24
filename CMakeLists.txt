cmake_minimum_required(VERSION 3.22)
project(huaray_camera)

set(CMAKE_CXX_STANDARD 11)
find_library( CAMERA_LIB NAMES MVSDK HINTS ./camera_lib/arm64 REQUIRED)
find_package( OpenCV REQUIRED )



include_directories(camera_include)

include_directories( ${OpenCV_INCLUDE_DIRS} )




add_executable(huaray_camera main.cpp Camera.cpp Camera.h)
message(STATUS ${CAMERA_LIB})
target_link_libraries(huaray_camera  ${CAMERA_LIB} ${OpenCV_LIBS} rt pthread)
