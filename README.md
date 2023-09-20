# CAN Bus

This is NFR's CAN Bus repository. Expect this to be a submodule in your own repository at some point. This might be merged into a larger shared repository with more than CAN code in the future.

That also means that be very careful when you're updating this repo because if you break something then you might break CAN for everyone. If you want to experiment, make your own branch and do your testing there, then submit a pull request (PR) to merge it back into main.

### Background

If you don't know what CAN is, read this document [here](https://docs.google.com/document/d/1XAJNA9vFf0h5ruzI_uM2yF3VfZlPSxpRNXcBMb-HSx4/edit?usp=sharing)

This project requires VS Code's PlatformIO extension to work. If you aren't familiar with PlatformIO or VS Code, see this setup tutorial [here](https://docs.google.com/document/d/1lHxgOpmPJfi5fyBfCM1aA54dtm3wqHcFUew8G-NeXwE/edit?usp=sharing)

To read and edit the CAN database (DBC file), use [Kvaser Database Editor 3](https://www.kvaser.com/download/)

If you only need to view the database, we have a GitHub Actions workflow that creates a .csv file in the docs/ folder which is automatically updated from the .dbc file

### Coverage

This code base is intended to offer CAN functionality for all the hardware we have on the car via a single interface. This allows for the particular hardware you're working on to be abstracted away and for uniform libraries for all our hardware to be written.

This code base currently includes the following hardware platforms:

- Teensy 4.0, 4.1
- ESP32

More support to come in the future.

We would also like to thank our sponsor [Innomaker](https://www.inno-maker.com/product/usb2can-cable/) for supporting us with their products for the 22-23 season!

### Updates

This code has support for uploading new code to the ESP32 over CAN. To use this feature, add
    upload_protocol = custom
    upload_can = y
to the env in platformio.ini, and also add
    [can_update]
    update_baud = 500000
    update_message_id = 0x530
to platformio.ini
