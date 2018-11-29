# MM-ownAdapters
Custom [Micro-manager](https://micro-manager.org/ "Micro-manager website") device adapters

This repository contains the code (device adapters) developped to control specific hardware with [Micro-manager](https://micro-manager.org/ "Micro-manager website"). 
The following were developped new:

* LaserQuantumLaser: this adapter allows controlling gem/ventus/opus/axiom series from LaserQuantum. For lasers that can be 
current controlled (factory enabled), the adapter permits switching control modes. This device adapter has been added to the Micro-Manager
source-code.
* MicroMojo: device adapter used for the [Micro-Mojo project](https://github.com/jdeschamps/MicroMojo "Micro-Mojo on GitHub").
This project uses a cheap FPGA to implement electronic control of several elements on a microscope (flexible laser triggering,
servos, PWM, TTL).
* SmarActHCU-3D: computer control of SmarAct HCU, SCU and CU controllers. This device adapter has been added to the Micro-Manager source code.
* ThorlabsElliptecSlider: Control of [ELL9](https://www.thorlabs.com/thorproduct.cfm?partnumber=ELL9) piezo 4-position slider from Thorlabs.
* Toptica-iBeamSmart: several device adapters to control different aspects of an iBeamSmart laser, depending on available options (obsolete: see iBeamSmartCW).
* Toptica-iBeamSmartCW: global device adapter for the iBeamSmart that makes use of a hidden control mode. This device adapter simplifies
the laser interface (only one channel) and detects automatically the possibility to trigger externally the laser.

Are also included device adapters whose scope was extended:

* Aladdin: The original device adapter from Kurt Thorn (UCSF, 2011) was extended to include the possibility to program the pumps
(using the Aladdin API). Most of the functions have been implemented.
* NI100X: The original code from Nenad Amodaj (100X Imaging Inc, 2010) was extended to Analog read (input).

For any further information, contact me: 'joran'.'deschamps'(at) 'embl'.'de' or through the [EMBL contact form](https://intranet.embl.de/research/cbb/ries/members/index.php?s_personId=CP-60017864).

Joran Deschamps, EMBL, 2018
