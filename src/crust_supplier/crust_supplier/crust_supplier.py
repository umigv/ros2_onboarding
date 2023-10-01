import rclpy
import random
from msgs_and_srvs.msg import Crust
from rclpy.node import Node

class crust_supplier(Node):
    def __init__(self):
        super().__init__('crust_supplier')
        self.publisher_ = self.create_publisher(Crust, 'crust', 10)
        timer_period =  1 # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)
    def timer_callback(self):
        msg = Crust()
        self.i = random.randint(1, 1000)
        if self.i % 3 == 0:
            self.order = '18'
            msg.crust_id = 113
        elif self.i % 3 == 1:
            self.order = '15'
            msg.crust_id = 127
        else:
            self.order = '12'
            msg.crust_id = 131
        msg.crust_id = int(self.order)
        msg.crust_id = msg.crust_id * self.i
        msg.ingredient = self.order + ' inch crust'
        self.publisher_.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    crust_pub = crust_supplier()
    rclpy.spin(crust_pub)
    crust_pub.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()