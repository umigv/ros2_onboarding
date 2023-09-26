from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='crust_supplier',
            namespace='crust_supplier',
            executable='crust_supplier',
            name='crust_sup_active'
        ),
        Node(
            package='toppings_supplier',
            namespace='toppings_supplier',
            executable='toppings_supplier',
            name='toppings_sup_active'
        ),
        Node(
            package='customer',
            namespace='customer',
            executable='customer',
            name='customer_active'
        ),
        Node(
            package='delivery_man',
            namespace='delivery_man',
            executable='delivery_man',
            name='delivery_man_active'
        )
    ])