To use scale add to the folder  

/etc/udev/rules.d/

the following file: 99-usb-scales.rules  which contains:

SUBSYSTEM=="usb", ATTR{idVendor}=="1446", ATTR{idProduct}=="6a73", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="7b7c", ATTR{idProduct}=="0100", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="2474", ATTR{idProduct}=="0550", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="2474", ATTR{idProduct}=="3550", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="0eb8", ATTR{idProduct}=="f000", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="6096", ATTR{idProduct}=="0158", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="0b67", ATTR{idProduct}=="555e", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="0922", ATTR{idProduct}=="8004", MODE="0776"
SUBSYSTEM=="usb", ATTR{idVendor}=="0922", ATTR{idProduct}=="8003", MODE="0776"