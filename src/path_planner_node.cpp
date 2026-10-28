#include <chrono>
#include <cmath>
#include <memory>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

struct Point2D {
  double x;
  double y;
};

class PathPlannerNode : public rclcpp::Node
{
public:
  PathPlannerNode() : Node("path_planner_node")
  {
    this->declare_parameter<double>("start_x", 0.0);
    this->declare_parameter<double>("start_y", 0.0);
    this->declare_parameter<double>("goal_x", 30.0);
    this->declare_parameter<double>("goal_y", 0.0);
    this->declare_parameter<double>("safety_margin", 1.2);
    this->declare_parameter<double>("step_size", 0.5);

    obstacle_sub_ = this->create_subscription<visualization_msgs::msg::MarkerArray>(
      "obstacles", 10, std::bind(&PathPlannerNode::obstacle_callback, this, _1));

    path_pub_ = this->create_publisher<nav_msgs::msg::Path>("planned_path", 10);

    timer_ = this->create_wall_timer(
      500ms, std::bind(&PathPlannerNode::replan_and_publish, this));

    RCLCPP_INFO(this->get_logger(), "Path Planner Node started. Waiting for obstacles...");
  }

private:
  void obstacle_callback(const visualization_msgs::msg::MarkerArray::SharedPtr msg)
  {
    obstacles_.clear();
    for (const auto & marker : msg->markers) {
      obstacles_.push_back({marker.pose.position.x, marker.pose.position.y});
    }
  }

  bool is_collision(double x, double y, double margin)
  {
    for (const auto & obs : obstacles_) {
      double dist = std::hypot(x - obs.x, y - obs.y);
      if (dist < margin) {
        return true;
      }
    }
    return false;
  }

  void replan_and_publish()
  {
    double start_x = this->get_parameter("start_x").as_double();
    double start_y = this->get_parameter("start_y").as_double();
    double goal_x = this->get_parameter("goal_x").as_double();
    double goal_y = this->get_parameter("goal_y").as_double();
    double margin = this->get_parameter("safety_margin").as_double();
    double step = this->get_parameter("step_size").as_double();

    nav_msgs::msg::Path path;
    path.header.stamp = this->now();
    path.header.frame_id = "map";

    double curr_x = start_x;
    double curr_y = start_y;

    while (curr_x < goal_x) {
      geometry_msgs::msg::PoseStamped pose;
      pose.header.frame_id = "map";
      pose.header.stamp = this->now();
      pose.pose.position.x = curr_x;
      pose.pose.position.y = curr_y;
      pose.pose.position.z = 0.1;
      pose.pose.orientation.w = 1.0;
      path.poses.push_back(pose);

      curr_x += step;

      // Akadálykikerülés reaktív logikával
      if (is_collision(curr_x, curr_y, margin)) {
        // Kerülés oldalirányba
        if (!is_collision(curr_x, curr_y + 1.5, margin)) {
          curr_y += 1.0;
        } else if (!is_collision(curr_x, curr_y - 1.5, margin)) {
          curr_y -= 1.0;
        } else {
          curr_y += 2.0;
        }
      } else {
        // Visszatérés a célérték egyenesére
        if (curr_y > goal_y + 0.1) {
          curr_y -= 0.2;
        } else if (curr_y < goal_y - 0.1) {
          curr_y += 0.2;
        }
      }
    }

    // Célpont hozzáadása
    geometry_msgs::msg::PoseStamped goal_pose;
    goal_pose.header.frame_id = "map";
    goal_pose.header.stamp = this->now();
    goal_pose.pose.position.x = goal_x;
    goal_pose.pose.position.y = goal_y;
    goal_pose.pose.position.z = 0.1;
    goal_pose.pose.orientation.w = 1.0;
    path.poses.push_back(goal_pose);

    path_pub_->publish(path);
  }

  rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr obstacle_sub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<Point2D> obstacles_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PathPlannerNode>());
  rclcpp::shutdown();
  return 0;
}
