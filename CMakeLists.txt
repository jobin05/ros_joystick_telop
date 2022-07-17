cmake_minimum_required(VERSION 3.0.2)
project(joystick_telop)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
)
catkin_package(

)
include_directories(

  ${catkin_INCLUDE_DIRS}
)
add_executable(${PROJECT_NAME}_node src/joystick_telop_node.cpp)
add_dependencies(${PROJECT_NAME}_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${PROJECT_NAME}_node ${catkin_LIBRARIES})
