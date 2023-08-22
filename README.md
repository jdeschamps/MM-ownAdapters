# MM-ownAdapters
Custom [Micro-manager](https://micro-manager.org/ "Micro-manager website") device adapters

This repository contains the code (device adapters) developped to control specific hardware with [Micro-manager](https://micro-manager.org/ "Micro-manager website"). 

The following device adapters are not included in Micro-Manager (as opposed to the others):

* **Aladdin**: The original device adapter from Kurt Thorn (UCSF, 2011) was extended to include the possibility to program the pumps
(using the Aladdin API). Most of the functions have been implemented.
* **NI100X**: The original code from Nenad Amodaj (100X Imaging Inc, 2010) was extended to add Analog read (input).

In addition, there is a modified PI device adapter (**PI_FocusLock**) to switch the sensors from internal to external. This is used within a focus-locking system to set specific parameter values in our E709 PI controller. **Before use, please contact PI to know the parameters required for your stage and compare them to the values set in PI_FocusLock.cpp.**  

## How to compile the adapters

Micro-Manager device adapters need to be build against particular versions of the MMDevice API.

Note: these instructions can potentially quickly be outdated.

- Clone https://github.com/micro-manager/mmCoreAndDevices
- Install Microsoft Visual C++ 2018 (v142)
- Place the device adapter folder in the devices folder of mmCoreAndDevices
- Open the .vcxproj of the device adapter
- In the Solution explorer, add an existing project: MMDevice-SharedRuntime.vcxproj (in MMDevice folder in mmCoreAndDevices)
- Right click on one of the projects and select build order. Make the device adapter you are interested in dependent on MMDevice-SharedRuntime
- In the top menu, set the build to Release and x64
- Build the device adapter
- Place the compiled mmgr_*.dll in your Micro-Manager installation 

For any further information, [open an issue in this repository](https://github.com/jdeschamps/MM-ownAdapters/issues).

Joran Deschamps, EMBL, 2018
