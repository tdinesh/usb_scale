#include "ros/ros.h"
#include <usb_scale/usb_scale.h>
#include <usb_scale/Scale.h>

class UsbScaleNode
{
public:
  UsbScaleNode();
  void publishScale();

private:
  ros::NodeHandle nh_;
  ros::Publisher scale_pub_;
  UsbScale sc_;
};

UsbScaleNode::UsbScaleNode()
{
  nh_ = ros::NodeHandle("~");
  scale_pub_ = nh_.advertise<usb_scale::Scale>("scale", 10);
  sc_.init();
}

void UsbScaleNode::publishScale()
{
  double weight;
  std::string units, msg;
  bool status = sc_.transfer_data(weight, units, msg);

  if(status)
  {
    usb_scale::Scale sl;
    sl.weight = weight;
    sl.units = units;
    sl.message = msg;
    scale_pub_.publish(sl);
  }
  else
    ROS_WARN("Could not read scale");
}


int main(int argc, char** argv)
{

  ros::init(argc, argv, "usb_scale");

  UsbScaleNode scn;

  ros::Rate loop_rate(5);

  while (ros::ok())
  {
    scn.publishScale();

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
};