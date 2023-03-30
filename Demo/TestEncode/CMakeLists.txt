
cmake_minimum_required(VERSION 2.8)

project(TestEncoder)

include_directories(${PROJECT_INCLUDE_DIR}/CameraSDK)

add_executable(TestEncoder TestEncoder.cpp)
target_link_libraries(TestEncoder CameraSDK)

add_executable(TestPlayView TestPlayView.cpp)
target_link_libraries(TestPlayView CameraSDK)

add_executable(TestTimer TestTimer.cpp)
target_link_libraries(TestTimer CameraSDK)