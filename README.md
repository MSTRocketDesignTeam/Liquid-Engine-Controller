<p align="center">
    <img src="assets/images/RDT_LOGO.png" width="60%" alt="Missouri S&T Rocket Design Team">
</p>

<p align="center"><sup><em>*This repo is intended for educational purposes only*</em></sup></p>

# SRAD Engine Controller
A student researched and designed (SRAD) engine controller for the liquid bipropellant rocket. 

## About
This project is the first iteration of the flight controller for the Rocket Design Team's (RDT) liquid bipropellant rocket.
It aims to provide basic engine control functionality, such as opening and closing of the servo ball valves (SBV), as well as to set the hardware baseline for more advanced control functionality like throttling algorithms for flight efficiency. Additionally, the Liquid Engine Controller (LEC) aims to serve as the data acquisition computer for the propulsion system, in order to analyze the performance of the propulsion system. 



## Documentation
*placeholder*

## Development 
*placeholder* 

## File Structure
This repo contains three main sections, hardware, firmware, and software.

### Hardware:
The hardware directory will contain all of the files related to the design and assembly of the PCBs. This includes KiCad projects as well as BOMs. 

### Firmware:
The firmware directory will contain all of the code (C++) necessary to operate the above hardware. Once the boards have been designed and assembled this is where significant effort will be made.

### Software: 
The software directory will contain the code (Python) for the LEC's computer interface. This program will be responsible for configuring the LEC prior to launch as well as for displaying live propulsion system state data from the launch pad. 

Future developments may allow for preflight simulations (FEA / CFD of engine, estimated throttling curve, etc.). 



