#include <chrono>
#include <cmath>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "visualization_msgs/msg/marker.hpp"

using namespace std::chrono_literals;

class PathPlannerNode : public rclcpp::Node {
public:
  PathPlannerNode() : Node("path_planner_node"), current_step_(0) {
    obstacle_sub_ = this->create_subscription<visualization_msgs::msg::MarkerArray>(
        "/obstacles", 10,
        std::bind(&PathPlannerNode::obstacle_callback, this, std::placeholders::_1));

    path_pub_ = this->create_publisher<nav_msgs::msg::Path>("/planned_path", 10);
    vehicle_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("/moving_vehicle", 10);

    // 20 Hz-es időzítő a jármű finom mozgásához
    timer_ = this->create_wall_timer(
        50ms, std::bind(&PathPlannerNode::move_vehicle_callback, this));

    RCLCPP_INFO(this->get_logger(), "Path planner node elindult animált járművel.");
  }

private:
  void obstacle_callback(const visualization_msgs::msg::MarkerArray::SharedPtr msg) {
    nav_msgs::msg::Path path;
    path.header.stamp = this->now();
    path.header.frame_id = "map";

    // Egyszerű útvonaltervezés akadálykerüléssel
    for (double x = 0.0; x <= 20.0; x += 0.2) {
      double y = 0.0;

      for (const auto &marker : msg->markers) {
        double dist_x = std::abs(marker.pose.position.x - x);
        if (dist_x < 1.0) {
          // Kerülés oldalra, ha akadály van a vonalban
          if (marker.pose.position.y >= 0.0) {
            y -= 1.0 * (1.0 - dist_x);
          } else {
            y += 1.0 * (1.0 - dist_x);
          }
        }
      }

      geometry_msgs::msg::PoseStamped pose;
      pose.header = path.header;
      pose.pose.position.x = x;
      pose.pose.position.y = y;
      pose.pose.position.z = 0.1;
      pose.pose.orientation.w = 1.0;
      path.poses.push_back(pose);
    }

    current_path_ = path;
    path_pub_->publish(path);
  }

  void move_vehicle_callback() {
    if (current_path_.poses.empty()) {
      return;
    }

    // Gömb marker pozíciójának frissítése a pályán
    visualization_msgs::msg::Marker vehicle;
    vehicle.header.stamp = this->now();
    vehicle.header.frame_id = "map";
    vehicle.ns = "vehicle";
    vehicle.id = 0;
    vehicle.type = visualization_msgs::msg::Marker::SPHERE;
    vehicle.action = visualization_msgs::msg::Marker::ADD;

    auto pose = current_path_.poses[current_step_].pose;
    vehicle.pose = pose;
    vehicle.pose.position.z = 0.3; // Kicsit lebegjen a vonal felett

    vehicle.scale.x = 0.6;
    vehicle.scale.y = 0.6;
    vehicle.scale.z = 0.6;

    // Élénk narancssárga szín
    vehicle.color.r = 1.0f;
    vehicle.color.g = 0.5f;
    vehicle.color.b = 0.0f;
    vehicle.color.a = 1.0f;

    vehicle_pub_->publish(vehicle);

    // Léptetés a következő pontra, a végén újraindul
    current_step_ = (current_step_ + 1) % current_path_.poses.size();
  }

  rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr obstacle_sub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_pub_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr vehicle_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  nav_msgs::msg::Path current_path_;
  size_t current_step_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PathPlannerNode>());
  rclcpp::shutdown();
  return 0;
}
