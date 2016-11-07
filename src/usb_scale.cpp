#include <iostream>
#include <math.h>

#include <usb_scale/usb_scale.h>

#define WEIGH_REPORT_SIZE 0x06

UsbScale::UsbScale()
{
  ctx_ = NULL;
  dev_handle_ = NULL;
  endpoint_address_ = LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE;
  max_packet_size_ = WEIGH_REPORT_SIZE;
  init_ = false;
}

UsbScale::~UsbScale()
{
  if(init_)
  {
    libusb_close(dev_handle_);
    libusb_exit(ctx_);
  }
}
bool UsbScale::init()
{
  ssize_t cnt; //holding number of devices in list
  libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices

  int r = libusb_init(&ctx_); //initialize a library session
  if(r < 0)
  {
    std::cout << "Init Error " << r << std::endl;
    return init_;
  }

  libusb_set_debug(ctx_, 3); //set verbosity level to 3, as suggested in the documentation

  cnt = libusb_get_device_list(ctx_, &devs); //get the list of devices

  std::cout << "Found usb devices, count " << cnt << std::endl; //there was an error

  if(cnt < 0)
  {
    std::cout << "Get Device Error" << std::endl; //there was an error
    return init_;
  }

  libusb_device* dev = NULL;

  dev = find_device(devs);

  if(dev == NULL)
  {
    std::cout << "No USB scale found on this computer" << std::endl;
    return init_;
  }

  r = libusb_open(dev, &dev_handle_);
  if(r < 0)
  {
    if(r == LIBUSB_ERROR_ACCESS)
      std::cout << "Permission denied to scale" << std::endl;
    else if(r == LIBUSB_ERROR_NO_DEVICE)
      std::cout << "Scale has been disconnected" << std::endl;

    return init_;
  }

  //Get the endpoint address
  struct libusb_config_descriptor *config;
  r = libusb_get_config_descriptor(dev, 0, &config);
  if (r == 0)
  {
    // assuming we have only one endpoint
    endpoint_address_ = config->interface[0].altsetting[0].endpoint[0].bEndpointAddress;
    max_packet_size_ = config->interface[0].altsetting[0].endpoint[0].wMaxPacketSize;
    std::cout << "max packet " << max_packet_size_ << std::endl;

    libusb_free_config_descriptor(config);
  }

  libusb_free_device_list(devs, 1); //free the list, unref the devices in it

  //find out if kernel driver is attached
  if(libusb_kernel_driver_active(dev_handle_, 0) == 1)
  {
    std::cout << "Kernel Driver Active" << std::endl;
    if(libusb_detach_kernel_driver(dev_handle_, 0) == 0) //detach it
      std::cout << "Kernel Driver Detached!" << std::endl;
  }

  //claim interface 0 (the first) of device
  r = libusb_claim_interface(dev_handle_, 0);
  if(r < 0)
  {
    std::cout<<"Cannot Claim Interface" << std::endl;
    return init_;
  }

  std::cout << "Claimed Interface" << std::endl;

  init_ = true;
  return init_;

}

libusb_device* UsbScale::find_device(libusb_device **devs)
{
  libusb_device* dev = NULL;

  // Loop through each USB device, and for each device, loop through the
  // scales list to see if it's one of our listed scales.
  int i =0;
  while ((dev = devs[i++]) != NULL)
  {

    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0)
    {
      std::cout << "failed to get device descriptor" << std::endl;
      return NULL;
    }

    for (int j = 0; j < NSCALES; j++)
    {
      if(desc.idVendor  == scales[j][0] && desc.idProduct == scales[j][1])
      {
        std::cout << "matched " << desc.idVendor << " " << desc.idProduct << std::endl;
        return dev;
      }
    }
  }
}

bool UsbScale::get_scale_data(unsigned char* dat, double &weight, std::string &units, std::string &msg)
{
  //Rip apart the scale's data packet according to *HID Point of Sale Usage Tables*.

  uint8_t report = dat[0];
  uint8_t status = dat[1];
  uint8_t unit   = dat[2];

  // Accoring to the docs, scaling applied to the data as a base ten exponent
  int8_t  expt   = dat[3];
  // convert to machine order at all times
  weight = (double) le16toh(dat[5] << 8 | dat[4]);
  // since the expt is signed, we do not need no trickery
  weight = weight * pow(10, expt);
  units = UNITS[unit];

  // The scale's first byte, its "report", is always 3.
  if(report != 0x03 && report != 0x04)
  {
    msg = "Error reading scale data";
    std::cout << msg << std::endl;
    return false;
  }

  // Switch on the status byte given by the scale. Note that we make a
  // distinction between statuses that we simply wait on, and statuses that
  // cause us to stop (`return -1`).
  switch(status)
  {
    case 0x01:
      msg = "Scale reports Fault";
      return false;
    case 0x02:
      msg = "Scale is zero'd...";
      break;
    case 0x03:
      std::cout << "Weighing..." << std::endl;
      break;
     // 0x04 is the only final, successful status, and it indicates that we
     // have a finalized weight ready to print. Here is where we make use of
     // the `UNITS` lookup table for unit names.
    case 0x04:
      msg = "Read successful";
      break;
    case 0x05:
      msg = "Scale reports Under Zero";
      break;
    case 0x06:
      msg = "Scale reports Over Weight";
      break;
    case 0x07:
      msg = "Scale reports Calibration Needed";
      break;
    case 0x08:
      msg = "Scale reports Re-zeroing Needed!";
      break;
    default:
      msg = "Unknown status code: ";
      return false;
  }

  return true;
}

bool UsbScale::transfer_data(double &weight, std::string &units, std::string &msg)
{
  if(!init_)
    return false;

  unsigned char data[max_packet_size_];
  int transferred;
  bool scale_result;

  // A `libusb_interrupt_transfer` of 6 bytes from the scale is the
  // typical scale data packet, and the usage is laid out in *HID Point
  // of Sale Usage Tables*, version 1.02.
  int r = libusb_interrupt_transfer(
        dev_handle_,
        //bmRequestType => direction: in, type: class,
         //    recipient: interface
        endpoint_address_,
        data,
        max_packet_size_, // length of data
        &transferred,
        10 //timeout => 0.01 sec
  );

  // If the data transfer succeeded, then we pass along the data we
  // received tot **print_scale_data**.
  if(r == 0)
  {
    scale_result = get_scale_data(data, weight, units, msg);
    //std::cout << "Weight " << weight << " " << units << " " << msg << std::endl;
    return true;
  }

  return false;
}