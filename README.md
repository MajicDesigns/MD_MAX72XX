# MAX72xx LED Matrix Display Library

The library implements functions that allow the MAX72xx to be used for LED matrices (64 individual LEDs), allowing the programmer to use the LED matrix as a pixel device, displaying graphics elements much like any other pixel addressable display.

In this scenario, it is convenient to abstract out the concept of the hardware device and create a uniform and consistent pixel address space, with the libraries determining device and device-element address. Similarly, control of the devices is uniform and abstracted to a system level.

The library still retains flexibility for device level control, should the developer require, through the use of overloaded class methods.

[Library Documentation](https://majicdesigns.github.io/MD_MAX72XX/)
