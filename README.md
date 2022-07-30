# CAN Bus
This is NFR's CAN Bus repository. Expect this to be a submodule in your own repository at some point. This might be merged into a larger shared repository with more than CAN code in the future. 

That also means that be very careful when you're updating this repo because if you break something then you might break CAN for everyone. If you want to experiment, make your own branch and do your testing there, then submit a pull request (PR) to merge it back into  main. 

### Background

If you don't know what CAN is, read this document [here](https://docs.google.com/document/d/1XAJNA9vFf0h5ruzI_uM2yF3VfZlPSxpRNXcBMb-HSx4/edit?usp=sharing)

### Coverage

This code base is intended to offer CAN functionality for all the hardware we have on the car via a single interface. This allows for the particular hardware you're working on to be abstracted away and for uniform libraries for all our hardware to be written. 

This code base currently includes the following hardware platforms: 
- Teensy 4.0, 4.1 
- ESP32

More support to come in the future. 