Firmware for the WheelECU
=========================

The firmware uses DAVE CE to configure and generate code for the hardware
abstraction. DAVE can be downloaded from Infineon's website, however it isn't
needed for just building.

Build
-----

Install dependencies on Ubuntu by running:

```
sudo apt-get install gcc-arm-none-eabi binutils-arm-none-eabi gdb-arm-none-eabi
```

Install Meddela by following the instructions in Meddela's README file:

https://github.com/CodileAB/meddela

Now run make to build:

```
make -j4 NODE_ID=0x11
```


