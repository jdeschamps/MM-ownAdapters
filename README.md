# MM-ownAdapters
Custom [Micro-manager](https://micro-manager.org/ "Micro-manager website") device adapters

This repository contains the code (device adapters) developped to control specific hardware with [Micro-manager](https://micro-manager.org/ "Micro-manager website"). 

The following device adapters are not included in Micro-Manager (as opposed to the others):

* **Aladdin**: The original device adapter from Kurt Thorn (UCSF, 2011) was extended to include the possibility to program the pumps
(using the Aladdin API). Most of the functions have been implemented.
* **NI100X**: The original code from Nenad Amodaj (100X Imaging Inc, 2010) was extended to add Analog read (input).
* **PI focus lock**: see next section

## PI focus lock

In addition, there is a modified PI device adapter (**PI_FocusLock**) to switch the sensors from internal to external. This is used within a focus-locking system to set specific parameter values in our E709 PI controller. 

⚠️ **Before use, please contact PI to know the parameters required for your stage and compare them to the values set in PI_FocusLock.cpp. The specific parameters changed are outlined here: https://github.com/jdeschamps/MM-ownAdapters/blob/main/DeviceAdapters/PI_FocusLock/README.md.** ⚠️

A compiled PI adapter can be found here: https://github.com/jdeschamps/MM-ownAdapters/releases

## How to compile the adapters

Micro-Manager device adapters need to be build against particular versions of the MMDevice API. That means that as time goes by, the device API changes and the device adapters need to be recompiled.

### Where is the device interface version defined?

The device interface is defined in the [mmCoreAndDevices](https://github.com/micro-manager/mmCoreAndDevices) repository, more specifically in [MMDevice.h](https://github.com/micro-manager/mmCoreAndDevices/blob/52af1c314f761116674c4600eedf6d1ece21a152/MMDevice/MMDevice.h#L30). Everytime this number changes, the next nightly build of Micro-Manager will contain device adapters compiled against the new version of the device adapter.

When it comes to custom device adapter, that is to say device adapters not integrated into Micro-Manager directly, this means that the device adapters must be recompiled for the newest device interface in order to be used with recent nightly builds.

Fortunately, the device interface does not change too often.

### How to compile the adapters for the newest device interface

Note: these instructions can potentially become outdated.

- Clone https://github.com/micro-manager/mmCoreAndDevices (or pull the latest version)
- Install Microsoft Visual C++ 2019 (v142)
- Place the device adapter folder in the devices folder of mmCoreAndDevices
- Open the .vcxproj of the device adapter
- Right click on the device adapter project and go to Properties. Check that the Platform Toolset is set to "Visual Studio 2019 (v142)" (it might change by clicking on it)
- In the Solution explorer, add an existing project by clicking on the Solution and selecting Add > Existing Project: MMDevice-SharedRuntime.vcxproj (in MMDevice folder in mmCoreAndDevices)
- Right click on one of the projects and select Build Dependencies > Project Dependencies. Make the device adapter you are interested in dependent on MMDevice-SharedRuntime
- In the top menu, set the build to Release and x64
- Build the device adapter
- Place the compiled mmgr_*.dll in your Micro-Manager installation 

For any further information, [open an issue in this repository](https://github.com/jdeschamps/MM-ownAdapters/issues).
