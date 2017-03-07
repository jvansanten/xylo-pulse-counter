# Xylo pulse counter

This project contains firmware and software for a TTL pulse counter based on
the [KNJN Xylo-EM development board](http://www.knjn.com/ShopBoards_USB2.html).
Here's a rundown of the components:

1. FPGA firmware: `Firmware/CountIntClk` contains design files and Altera RBF
file to program the FPGA. `Firmware/CountIntClk/testinggo_all.rbf` should be
programmed into the board's boot-ROM using the closed-source (and Windows-only)
`FPGAconf` tool provided with the KNJN board.

2. FX2 firmware: `cycfx2prog/xylo_setup.c` is a program for the 8051-based
Cypress FX2 chip on the Xylo board that implements the USB interface, and is
downloaded to the board after power-on reset. It connects the USB endpoint
FIFOs to the FPGA and then gets out of the way. The associated bytecode is
included in the driver, but the source is a useful reference.

3. Host driver: `cycfx2prog/cycfx2prog` contains a command-line and a Python
client that download the firmware to the FX2 and communicates with the FPGA
over USB.

## Quickstart

To build the driver, you will need:

* CMake (> 3.1)
* A c++14-compliant compiler
* Python

After unpacking the source, build `cmake . && make`. The script
`test_pulsecounter.py` illustrates how to use the Python interface.

## Acknowledgements

The FPGA design was adapted from Joffrey Peter and Sergey Polyakov's [Multichannel Acquisition Board](https://www.nist.gov/services-resources/software/simple-and-inexpensive-fpga-based-fast-multichannel-acquisition-board).

The FX2 firmware is based on Brent Baccala's [saxo_loader](http://www.freesoft.org/software/saxo/).

The host driver is based on Wolfgang Wieser's [cycfx2prog](http://www.triplespark.net/elec/periph/USB-FX2/software/).

