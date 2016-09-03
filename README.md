# AirTop Front Panel Service daemon
# and GPU Thermal daemon

Copyright (c) 2015-2016, CompuLab ltd.  
Author: Andrey Gelman <<andrey.gelman@compulab.co.il>>  
License: GNU GPLv2 or later, at your option  

I am no more associated with CompuLab, so any difference between the latest code in this repository and CompuLab's repository should be considered normal course of events.

## Project submodules
This project consists of the following parts:
1. airtop-fpsvc - AirTop Front Panel service daemon  
    This service provides hardware-related metrics to the AirTop front panel
    controller.

1. gpu-thermald - GPU Thermal daemon  
    This service sets GPU temperature upper limit by constantly controlling
    its power.

1. There is also a *sister-project* - **package**, that wraps the services above into
debian packages:  
- airtop-fpsvc_upstart.deb
- airtop-fpsvc_systemd.deb

Additional package types might be added in the future.

## Building the source code
Issue *make* in project top directory (where Makefile is present).  
For debugging purposes, issue *make debug=1*.  
*obj* directory will hold object and other temporary files  
*bin* directory will hold the executables

## Supported features
- CPU Temperature
- CPU Frequency
- GPU Temperature
- HDD Temperature and volume - currently there is no FP request for HDD volume
- Limiting GPU power and/or temperature - GPU Thermal daemon

## Software design overview
### AirTop Front Panel Service daemon  
This daemon is quite robust and scalable in the expense of simplicity of
implementation.  
The daemon is built around 2 thread pools - frontend and backend.  
Frontend is a single-threaded 'owner' of the communication to and from the
Front Panel controller. It accepts requests from the FP, and sends back the
responses.  
Backend is a multi-threaded pool performing the tasks dispatched by the
frontend. Data acquired by the backend is handed over to frontend to be
passed on to the FP controller.  
Each thread is guarded by a watchdog, so that no task can spend in the
processing more than a preset time. Watchdog timeout is considered a major
failure, and leads to daemon restart.  
Notice, that this design enables adding additional frontends, e.g.
a web-based one, that would allow creation of web-based front panel useful
for system administration.
### GPU Thermal daemon
This daemon implements a modified PID control loop, limiting the GPU
temperature by timely limiting of GPU power.  
While the hard-coded PID factors were matched to meet AirTop case heat
dissipation, the whole implementation is not AirTop-specific, and is able of
serving any Nvidia GPU capable of power management.

