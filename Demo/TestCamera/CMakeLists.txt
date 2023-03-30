
cmake_minimum_required(VERSION 2.8)

project(TestCamera)

include_directories(${PROJECT_INCLUDE_DIR}/CameraSDK)

add_executable(TestCamera ../gstDisplay.cpp TestCamera.cpp)

target_link_libraries(TestCamera CameraSDK)