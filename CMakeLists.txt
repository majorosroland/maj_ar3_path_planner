cmake_minimum_required(VERSION 3.8)
project(maj_ar3_path_planner)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

# Include directories
include_directories(include)

# Executable: Obstacle Generator Node
add_executable(obstacle_generator_node src/obstacle_generator_node.cpp)
ament_target_dependencies(obstacle_generator_node
  rclcpp
  geometry_msgs
  visualization_msgs
)

# Executable: Path Planner Node
add_executable(path_planner_node src/path_planner_node.cpp)
ament_target_dependencies(path_planner_node
  rclcpp
  geometry_msgs
  nav_msgs
  visualization_msgs
)

# Install targets
install(TARGETS
  obstacle_generator_node
  path_planner_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install directories (launch, config)
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
