# twist_joystick

This is an example project which allows to send twist messages from an analog joystick to turtlesim
The code can be modified according to application.

------------

Sample video [available here](https://twitter.com/SameerT009/status/1497953393125605379?s=20&t=-jHgqTqu4rCb2s6nTARx3w "available here").

------------

### List of components used:
- [ESP 32](https://www.az-delivery.de/en/products/esp32-developmentboard "ESP 32")
- [Joystick](https://www.adafruit.com/product/512 "Joystick") 

------------

### About
This example uses a joystick connected to esp32 board. The analog reading from joystick are sent over ROS2 via micro-ros. The values are interpolated to match the required output.
In ROS2, these values are published to turtle1/cmd_vel topic to demonstrate the control of turtlesim with this example.

------------

### Tutorial
1. Follow the  [tutorial here](https://link.medium.com/pdmyDUIh9nb "tutorial here") to setup the micro-ros environment.
2. Clone this repository and copy the folder 'twist_joystick' inside your workspace such that folder structure is as  [YOUR WORKSPACE]/firmware/freertos_apps/apps/twist_joystick 
3. Open terminal can follow these commands:
- ros2 run micro_ros_setup configure_firmware.sh twist_joystick -t udp -i [LOCAL MACHINE IP ADDRESS] -p 8888
- ros2 run micro_ros_setup build_firmware.sh
- ros2 run micro_ros_setup flash_firmware.sh
4. Once the flashing is successful, run
- ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
5. Open new terminal window, and check ros topic
- ros2 topic echo /turtle1/cmd_vel
6. Open another terminal window and run turtlesim. If turtlesim package is not installed follow [tutorial here](https://docs.ros.org/en/foxy/Tutorials/Turtlesim/Introducing-Turtlesim.html#start-turtlesim "tutorial here").
- ros2 run turtlesim turtlesim_node

------------

### Connections
|  ESP32  |  Analog Joysitck  |
|  ------------ |  ------------ |
|  G32  |  H (x axis)  |
|  G33  |  V (y axis)  |
|  3.3V  |  VCC  |
|  GND  |  GND  |

------------

### Twist message inputs
|  ESP32  |  ROS2 Topic  |
|  ------------ |  ------------ |
|  V (y axis)  |  twist.linear.x  |
|  H (x axis)  |  twist.angular.z  |