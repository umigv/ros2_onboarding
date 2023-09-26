import rclpy
import random
from rclpy.node import Node
from msgs_and_srvs.srv import Order, Payment

class customer(Node):
    def __init__(self):
        super().__init__('customer')
        self.client_ = self.create_client(Order, 'order')
        self.service_ = self.create_service(Payment, 'payment', self.payment_callback)
        # while not self.client_.wait_for_service(timeout_sec=2):
        #     self.get_logger().info('service not available, waiting again...')
        self.order_list = {0: 0}
        self.req = Order.Request()
    def send_request(self):
        self.i = random.randint(0,1000)
        self.expected_id = 0
        self.pay = 0
        if self.i % 3 == 0:
            self.req.order = '18 inch pizza'
            self.expected_id = 113
            self.pay = 20
        elif self.i % 3 == 1:
            self.req.order = '15 inch pizza'
            self.expected_id = 127
            self.pay = 15
        else:
            self.req.order = '12 inch pizza'
            self.expected_id = 131
            self.pay = 10
        if self.i % 5 == 0:
            self.req.order += ' with pepperoni'
            self.expected_id *= 139
            self.pay += self.i * 0.007
        self.future = self.client_.call_async(self.req)
        rclpy.spin_until_future_complete(self, self.future)
        self.val = self.future.result()
        self.order_list[self.val.pizza_id] = 0
        if self.future % self.expected_id == 0:
            self.order_list[self.val.pizza_id] = self.pay 
        
    def payment_callback(self, request, response):
        if request.pizza_id not in self.order_list.keys():
            response.payment = 0.0
            return response
        else:
            response.payment = float(self.order_list[request.pizza_id])
            return response









def main(args=None):
    rclpy.init(args=args)
    cust = customer()
    cust.send_request()
    while rclpy.ok():
        rclpy.spin_once(cust)
    cust.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()