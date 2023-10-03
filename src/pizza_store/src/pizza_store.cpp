#include "rclcpp/rclcpp.hpp"
#include "rclcpp/node.hpp"
//#include "rclcpp_action/rclcpp_action.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include "msgs_and_srvs/msg/crust.hpp"
#include "msgs_and_srvs/msg/toppings.hpp"
#include "msgs_and_srvs/srv/order.hpp"
#include "msgs_and_srvs/srv/payment.hpp"
#include "msgs_and_srvs/action/delivery.hpp"

// using Delivery = msgs_and_srvs::action::Delivery;
// using DeliveryGoalHandle = rclcpp_action::Client<msgs_and_srvs::action::Delivery>;

class pizza_store : public rclcpp::Node {
    public:
        // The constructor should initialize the Node by giving it a name, "pizza_store".
        // Then we should create all of our subscribers, clients, action clients, ect.
        // rclcpp::QoS(rclcpp::KeepLast(10)).reliable()
        explicit pizza_store() :
            Node("pizza_store"), count_(0){
            crust_sub = this->create_subscription<msgs_and_srvs::msg::Crust>("/crust", 10, std::bind(&pizza_store::crust_callback, this, std::placeholders::_1));
            toppings_sub = this->create_subscription<msgs_and_srvs::msg::Toppings>("/toppings", 10, std::bind(&pizza_store::toppings_callback, this, std::placeholders::_1));
            order_srv = this->create_service<msgs_and_srvs::srv::Order>("order", std::bind(&pizza_store::order_srv_callback, this, std::placeholders::_1, std::placeholders::_2));
            payment_client = this->create_client<msgs_and_srvs::srv::Payment>("payment");
            //delivery_client = rclcpp_action::create_client<msgs_and_srvs::action::Delivery>(this, "delivery_man");
        }
        void run(){
            while(rclcpp::ok()){
                rclcpp::spin_some(shared_from_this());
                if(ordered){
                    request_payment();
                    ordered = false;
                }
                sleep(1);
            }
        }
    private:
        // void send_delivery(){
        //     if(!this->delivery_client->wait_for_action_server()){
        //         RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
        //         rclcpp::shutdown();
        //     }
        //     auto deliv_msg = Delivery::Goal();
        //     deliv_msg.pizza = "18 inch pizza with pepperoni, sausage, green pepper, banana peppers, and breadsticks.";
        //     auto send_goal_options = DeliveryGoalHandle::SendGoalOptions();
        //     send_goal_options.goal_response_callback = std::bind(&pizza_store::goal_callback, this, std::placeholders::_1);

        // }


        // void goal_callback(const DeliveryGoalHandle::SharedPtr & goal_handle)
        // {
        //     if (!goal_handle) {
        //     RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
        //     } else {
        //     RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
        //     }
        // }


        // void feedback_callback(
        //     DeliveryGoalHandle::SharedPtr,
        //     const std::shared_ptr<const Delivery::Feedback> feedback)
        // {
        //     if(feedback->patience == 0){
        //         auto cancel = DeliveryGoalHandle::CancelRequest();
        //         this->delivery_client->async_cancel_all_goals();   
        //     }
        // }




        // This function is the implementation of our payment_client.        
        void request_payment(){
            if(current_pizza_id == 0) { return; }
            auto request = std::make_shared<msgs_and_srvs::srv::Payment::Request>();
            request->pizza_id = current_pizza_id;
            while(!payment_client->wait_for_service()){
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Customer payment service not available, waiting again...");
            }
            auto result = payment_client->async_send_request(request);
            // Now we need to wait for the result.
            if (rclcpp::spin_until_future_complete(shared_from_this(), result) == rclcpp::FutureReturnCode::SUCCESS){
                // Can only access the result inside of this if statement.
                // The result needs to have been associated with SUCCESS, otherwise will throw error.
                // Can only access the future once to get the payment. Only use result.get() once or you will recieve std::future errors.
                double res = result.get()->payment;
                total_money += res;
                RCLCPP_INFO_STREAM(this->get_logger(), "Made $" << res << " from the last pizza. Total revenue: $" << total_money);
            } else {
                RCLCPP_ERROR_STREAM(this->get_logger(), "Error calling payment service.");
            }
            return;
        }
        // This is the callback function for the crust supplier.
        // The data we are subscribing to will be stored in msg.
        // Take the data from msg, and store it.
        void crust_callback(const msgs_and_srvs::msg::Crust &msg){
            //Include this for Debug: if(count_ % 10 == 0) {RCLCPP_INFO_STREAM(this->get_logger(), "crust_callback" << ": " << msg.ingredient);}
            crust_supply[msg.ingredient] = msg.crust_id;
            return;
        }
        // This is the callback function for the toppings supplier.
        // The data we are subscribing to will be stored in msg.
        // Take the data from msg, do what is needed with it, and store it.
        void toppings_callback(const msgs_and_srvs::msg::Toppings &msg){
            //Include this for Debug: if(count_ % 10 == 0) {RCLCPP_INFO_STREAM(this->get_logger(), "toppings_callback : " << msg.ingredient);} 
            toppings_supply[msg.ingredient] = msg.toppings_id;
            return;
        }
        void order_srv_callback(const std::shared_ptr<msgs_and_srvs::srv::Order::Request> req, std::shared_ptr<msgs_and_srvs::srv::Order::Response> res){
            //RCLCPP_INFO_STREAM(this->get_logger(), "2");
            std::string crust = req->order;
            crust = crust.substr(0, 13);
            std::string pine = "pineapple";
            std::string pepp = "pepperoni";
            std::string topping = "";
            if (req->order.find(pine)){ topping = pine; }
            if (req->order.find(pepp)){ topping = pepp; }
            if(((crust_supply.find(crust)) == nullptr) || ((toppings_supply.find(topping)) == nullptr)){
                res->pizza = "";
                res->pizza_id = 0;
                return;
            }
            auto citer = crust_supply.find(crust);
            auto topiter = toppings_supply.find(topping);
            current_pizza_id = citer->second * topiter->second;
            res->pizza = crust + topping;
            res->pizza_id = current_pizza_id;
            ordered = true;
            return;
        }
        // Create a subscriber that takes in a message of type: msgs_and_srvs::msg::Crust 
        rclcpp::Subscription<msgs_and_srvs::msg::Crust>::SharedPtr crust_sub;
        // Create a subscriber that takes in a message of type: msgs_and_srvs::msg::Toppings 
        rclcpp::Subscription<msgs_and_srvs::msg::Toppings>::SharedPtr toppings_sub;
        // Create a service that can be called with a service of type: msgs_and_srvs::srv::Order 
        rclcpp::Service<msgs_and_srvs::srv::Order>::SharedPtr order_srv;
        // Create a client that can call a service of type: msgs_and_srvs::srv::Payment 
        rclcpp::Client<msgs_and_srvs::srv::Payment>::SharedPtr payment_client;
        // Create an action client to call the delivery_man action
        //DeliveryGoalHandle::SharedPtr delivery_client;
        size_t count_;
        std::unordered_map<std::string, int64_t> crust_supply;
        std::unordered_map<std::string, int64_t> toppings_supply;
        int current_pizza_id = 0;
        double total_money;
        bool ordered;
};

int main(int argc, char** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<pizza_store>();
    node->run();
    rclcpp::shutdown();
    return 0;
}