cmake_minimum_required(VERSION 2.8.3)
project(usb_scale)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(catkin REQUIRED roscpp message_generation)

add_message_files(
  FILES
  Scale.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

find_package(libusb-1.0 REQUIRED)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES usb_scale
  CATKIN_DEPENDS message_runtime
#  DEPENDS system_lib
)



include_directories(include ${catkin_INCLUDE_DIRS} ${LIBUSB_1_INCLUDE_DIRS})

add_library(usb_scale
   src/usb_scale.cpp
)

add_executable(usb_scale_node src/usb_scale_node.cpp)

## Specify libraries to link a library or executable target against
target_link_libraries(usb_scale_node usb_scale
   ${catkin_LIBRARIES}
   ${LIBUSB_1_LIBRARIES}
)