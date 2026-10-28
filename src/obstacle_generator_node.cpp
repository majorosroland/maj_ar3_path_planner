#include <chrono>
#include <memory>
#include <random>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "visualization_msgs/msg/marker.hpp"

using namespace std::chrono_literals;

class ObstacleGeneratorNode : public rclcpp::Node
{
public:
  ObstacleGeneratorNode() : Node("obstacle_generator_node")
  {
    this->declare_parameter<int>("num_obstacles", 15);
    this->declare_parameter<double>("area_size_x", 30.0);
    this->declare_parameter<double>("area_size_y", 20.0);
    this->declare_parameter<double>("obstacle_radius", 0.8);

    marker_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(
      "obstacles", 10);

    generate_obstacles();

    timer_ = this->create_wall_timer(
      1000ms, std::bind(&ObstacleGeneratorNode::publish_obstacles, this));

    RCLCPP_INFO(this->get_logger(), "Obstacle Generator Node initialized.");
  }

private:
  void generate_obstacles()
  {
    int num_obstacles = this->get_parameter("num_obstacles").as_int();
    double area_x = this->get_parameter("area_size_x").as_double();
    double area_y = this->get_parameter("area_size_y").as_double();
    double radius = this->get_parameter("obstacle_radius").as_double();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_x(2.0, area_x - 2.0);
    std::uniform_real_distribution<> dis_y(-area_y / 2.0 + 1.0, area_y / 2.0 - 1.0);

    markers_.markers.clear();

    for (int i = 0; i < num_obstacles; ++i) {
      visualization_msgs::msg::Marker marker;
      marker.header.frame_id = "map";
      marker.ns = "trees";
      marker.id = i;
      marker.type = visualization_msgs::msg::Marker::CYLINDER;
      marker.action = visualization_msgs::msg::Marker::ADD;

      marker.pose.position.x = dis_x(gen);
      marker.pose.position.y = dis_y(gen);
      marker.pose.position.z = 1.0;
      marker.pose.orientation.w = 1.0;

      marker.scale.x = radius * 2.0;
      marker.scale.y = radius * 2.0;
      marker.scale.z = 2.0;

      // Zöld szín (fa/akadály)
      marker.color.r = 0.13f;
      marker.color.g = 0.55f;
      marker.color.b = 0.13f;
      marker.color.a = 0.9f;

      markers_.markers.push_back(marker);
    }
  }

  void publish_obstacles()
  {
    for (auto & marker : markers_.markers) {
      marker.header.stamp = this->now();
    }
    marker_pub_->publish(markers_);
  }

  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  visualization_msgs::msg::MarkerArray markers_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ObstacleGeneratorNode>());
  rclcpp::shutdown();
  return 0;
}
