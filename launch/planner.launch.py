import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    pkg_dir = get_package_share_directory('maj_ar3_path_planner')
    rviz_config = os.path.join(pkg_dir, 'config', 'planner.rviz')

    return LaunchDescription([
        Node(
            package='maj_ar3_path_planner',
            executable='obstacle_generator_node',
            name='obstacle_generator',
            output='screen'
        ),
        Node(
            package='maj_ar3_path_planner',
            executable='path_planner_node',
            name='path_planner',
            output='screen'
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        )
    ])
