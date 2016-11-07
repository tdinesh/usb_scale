#ifndef USB_SCALE_H_
#define USB_SCALE_H_

#include <libusb-1.0/libusb.h>

#define NSCALES 9

// Scales
uint16_t scales[NSCALES][2] = {\
    // Stamps.com Model 510 5LB Scale
    {0x1446, 0x6a73},
    // USPS (Elane) PS311 "XM Elane Elane UParcel 30lb"
    {0x7b7c, 0x0100},
    // Stamps.com Stainless Steel 5 lb. Digital Scale
    {0x2474, 0x0550},
    // Stamps.com Stainless Steel 35 lb. Digital Scale
    {0x2474, 0x3550},
    // Mettler Toledo
    {0x0eb8, 0xf000},
    // SANFORD Dymo 10 lb USB Postal Scale
    {0x6096, 0x0158},
    // Fairbanks Scales SCB-R9000
    {0x0b67, 0x555e},
    // Dymo-CoStar Corp. M25 Digital Postal Scale
    {0x0922, 0x8004},
    // DYMO 1772057 Digital Postal Scale
    {0x0922, 0x8003}
};

const char* UNITS[13] = {
    "units",        // unknown unit
    "mg",           // milligram
    "g",            // gram
    "kg",           // kilogram
    "cd",           // carat
    "taels",        // lian
    "gr",           // grain
    "dwt",          // pennyweight
    "tonnes",       // metric tons
    "tons",         // avoir ton
    "ozt",          // troy ounce
    "oz",           // ounce
    "lbs"           // pound
};

class UsbScale
{
public:
  UsbScale();
  ~UsbScale();
  bool init();
  bool transfer_data(double &weight, std::string &units, std::string &msg);

private:
  libusb_device* find_device(libusb_device **devs);
  bool get_scale_data(unsigned char* dat, double &weight, std::string &unit, std::string &msg);

  libusb_context *ctx_;
  libusb_device_handle* dev_handle_;
  uint8_t endpoint_address_;
  uint16_t max_packet_size_;

  bool init_;
};

#endif