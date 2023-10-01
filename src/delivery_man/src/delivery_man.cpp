#include <functional>
#include <memory>
#include <thread>
#include <string>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "msgs_and_srvs/action/delivery.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"


class DeliveryMan : public rclcpp::Node{
public:
  using Delivery = msgs_and_srvs::action::Delivery;
  explicit DeliveryMan(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("delivery_man", options){
    using namespace std::placeholders;
    this->action_server_ = rclcpp_action::create_server<Delivery>(
      this,
      name.c_str(),
      std::bind<rclcpp_action::GoalResponse>(&DeliveryMan::handle_goal, this, _1, _2),
      std::bind<rclcpp_action::CancelResponse>(&DeliveryMan::handle_cancel, this, _1),
      std::bind<void>(&DeliveryMan::handle_accepted, this, _1));
  }
private:
  std::string name = "delivery_man";
  rclcpp_action::Server<Delivery>::SharedPtr action_server_;
  
  rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const Delivery::Goal> goal){
    std::string pizza = goal->pizza;
    RCLCPP_INFO(this->get_logger(), "Received pizza to deliver: %s", pizza.c_str());
    (void)uuid;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }
  
  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<msgs_and_srvs::action::Delivery>> goal_handle){
    RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
    (void)goal_handle;
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<msgs_and_srvs::action::Delivery>> goal_handle){
    using namespace std::placeholders;
    // this needs to return quickly to avoid blocking the executor, so spin up a new thread
    std::thread{std::bind(&DeliveryMan::execute, this, _1), goal_handle}.detach();
  }

  void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<msgs_and_srvs::action::Delivery>> goal_handle){
    RCLCPP_INFO(this->get_logger(), "Executing goal");
    rclcpp::Rate loop_rate(1);

    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Delivery::Feedback>();
    auto result = std::make_shared<Delivery::Result>();
    int val = rand();
    for(int i = 0; i < 5; i++){
      if (val % 20 == 0){
        while(true){
          feedback->patience = 0;
          goal_handle->publish_feedback(feedback);
          loop_rate.sleep();
        }
        val = rand();
        loop_rate.sleep();
      }
    }
    result->payment = (20 + (val % 15)) * 1.06;
    if(rclcpp::ok()){
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "Pizza Delivered!");
    }
  }
};


int main(int argc, char **argv){
  rclcpp::init(argc, argv);
  auto action_server = std::make_shared<DeliveryMan>();
  rclcpp::spin(action_server);
  rclcpp::shutdown();
  return 0;
}