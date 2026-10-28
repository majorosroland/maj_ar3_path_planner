from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # Akadálygeneráló node
        Node(
            package='maj_ar3_path_planner',
            executable='obstacle_generator_node',
            name='obstacle_generator_node',
            output='screen',
            parameters=[{
                'num_obstacles': 15,
                'area_size_x': 30.0,
                'area_size_y': 20.0,
                'obstacle_radius': 0.8
            }]
        ),
        # Útvonaltervező node
        Node(
            package='maj_ar3_path_planner',
            executable='path_planner_node',
            name='path_planner_node',
            output='screen',
            parameters=[{
                'start_x': 0.0,
                'start_y': 0.0,
                'goal_x': 30.0,
                'goal_y': 0.0,
                'safety_margin': 1.2,
                'step_size': 0.5
            }]
        ),
        # RViz2 megjelenítő
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', ''],
            output='screen'
        )
    ])
