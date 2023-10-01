import rclpy
from rclpy.node import Node
from msgs_and_srvs.msg import Toppings 
from random import randint

class top_sup (Node):
    def __init__(self):
        super().__init__('toppings_supplier')
        self.publisher_ = self.create_publisher(Toppings, 'toppings', 10)
        timer_period = 0.1 # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback) 
        self.i = 0 # This is a counter for the callback function.
    def timer_callback(self):
        msg = Toppings()
        r = randint(1,1000)
        if (r % 66) == 0:
            msg.ingredient = 'pineapple'
            msg.toppings_id = randint(1,1000) * 307
        else:
            msg.ingredient = 'pepperoni'
            msg.toppings_id = randint(1,1000) * 139
        self.publisher_.publish(msg)
        self.i += 1


def main(args=None):
    rclpy.init(args=args)
    top_pub = top_sup()
    rclpy.spin(top_pub)
    top_pub.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()