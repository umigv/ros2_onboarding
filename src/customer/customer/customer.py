import rclpy
import random
import time
from rclpy.node import Node
from msgs_and_srvs.srv import Order, Payment

class customer(Node):
    def __init__(self):
        super().__init__('customer')
        self.client_ = self.create_client(Order, 'order')
        while not self.client_.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('service not available, waiting again...')
        self.service_ = self.create_service(Payment, 'payment', self.payment_callback)
        self.order_list = {0: 0}
        self.req = Order.Request()
    def send_request(self):
        self.i = random.randint(0,1000)
        self.expected_id = 0
        self.pay = 0
        if self.i % 3 == 0:
            self.req.order = '18 inch crust'
            self.expected_id = 113
            self.pay = 20
        elif self.i % 3 == 1:
            self.req.order = '15 inch crust'
            self.expected_id = 127
            self.pay = 15
        else:
            self.req.order = '12 inch crust'
            self.expected_id = 131
            self.pay = 10
        if self.i % 50 == 0:
            self.req.order += ' with pineapple'
            self.expected_id *= 307
            self.pay += self.i * 0.17
        else:
            self.req.order += ' with pepperoni'
            self.expected_id *= 139
            self.pay += self.i * 0.07
        self.future = self.client_.call_async(self.req)
        self.get_logger().info('calling client')
        rclpy.spin_until_future_complete(self, self.future)
        self.val = self.future.result()
        self.get_logger().info(self.val.pizza)
        self.get_logger().info(str(self.val.pizza_id))
        self.order_list[self.val.pizza_id] = 0
        if self.val.pizza_id % self.expected_id == 0:
            self.order_list[self.val.pizza_id] = self.pay 
        
    def payment_callback(self, request, response):
        self.get_logger().info(str(request.pizza_id))
        if request.pizza_id not in self.order_list:
            response.payment = 0.0
            self.get_logger().info('1')
        else:
            response.payment = float(self.order_list[request.pizza_id])
            self.get_logger().info('2')
        self.get_logger().info('Incoming request: %i payment: %f' % (request.pizza_id, response.payment))
        return response


def main(args=None):
    rclpy.init(args=args)
    cust = customer()
    while rclpy.ok():
        cust.send_request()
        rclpy.spin_once(cust)
        time.sleep(2)
    cust.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()