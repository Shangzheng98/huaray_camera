cmake_minimum_required(VERSION 3.22)
project(huaray_camera)

set(CMAKE_CXX_STANDARD 11)
#find_library( MVSDK HINTS ./camera_lib/ REQUIRED)

#find_library(CAMERA_LIB_1 rt HINTS ./camera_lib/GenICam/bin/Linux64_x64/)
find_package( OpenCV REQUIRED )


include_directories(camera_include)

include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable(huaray_camera main.cpp Camera.cpp Camera.h)

#link_directories(./camera_lib/x86/)
message(${CAMERA_LIB})
target_link_libraries(huaray_camera  ${CAMERA_LIB} ${OpenCV_LIBS} rt)