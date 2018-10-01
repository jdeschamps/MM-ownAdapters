//-----------------------------------------------------------------------------
// FILE:          iBeam-smart.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Controls iBeam smart laser series from Toptica through serial port
// COPYRIGHT:     EMBL
// LICENSE:       LGPL
// AUTHOR:        Joran Deschamps, 2018
//-----------------------------------------------------------------------------


#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/ModuleInterface.h"

#include <string>
#include <map>
#include <iomanip>
#include <iostream>

//-----------------------------------------------------------------------------
// Error code
//-----------------------------------------------------------------------------

#define ERR_PORT_CHANGE_FORBIDDEN    101
#define LASER_WARNING    102
#define LASER_ERROR    103
#define LASER_FATAL_ERROR    104
#define ADAPTER_POWER_OUTSIDE_RANGE    105
#define ADAPTER_PERC_OUTSIDE_RANGE    106
#define ADAPTER_ERROR_DATA_NOT_FOUND    107
#define ADAPTER_CANNOT_CHANGE_CH2_EXT_ON    108
#define LASER_CLIP_FAIL   109
#define ADAPTER_UNEXPECTED_ANSWER   110

//-----------------------------------------------------------------------------

// ------------------------------------
// Complete iBeamSmart laser controller
// with channels 1 and 2, fine and ext
// trigger options.
// ------------------------------------

class iBeamSmart: public CGenericBase<iBeamSmart>
{
public:
    iBeamSmart();
    ~iBeamSmart();

    // MMDevice API
    int Initialize();
    int Shutdown();

    void GetName(char* pszName) const;
    bool Busy();

	// getters
	int getMaxPower(int* maxpower);
	int getPower(int channel, double* power);
	int getChannelStatus(int channel, bool* status);
	int getFineStatus(bool* status);
	int getFinePercentage(char fine, double* value);
	int getExtStatus(bool* status);
	int getLaserStatus(bool* status);
	int getSerial(std::string* serial);
	int getFirmwareVersion(std::string* version);
	int getClipStatus(std::string* status);

	// setters
	int setLaserOnOff(bool b);
	int enableChannel(int channel, bool enable);
	int setPower(int channel, double pow);
	int setFineA(double perc);
	int setFineB(double perc);
	int enableExt(bool b);
	int enableFine(bool b);
	int setPromptOff();
	int setTalkUsual();

    // action properties
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnLaserOnOff(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerCh1(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerCh2(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableExt(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableCh1(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableCh2(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableFine(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFineA(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFineB(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnClip(MM::PropertyBase* pProp, MM::ActionType eAct);
	
	// convenience function 
	bool isError(std::string answer);
	bool isOk(std::string answer);
	int getError(std::string error);
	int publishError(std::string error);
	std::string to_string(double x);

private:
	std::string port_;
	std::string serial_;
	std::string clip_;    
	bool initialized_;
	bool busy_;
	bool laserOn_;
	bool fineOn_;
	bool ch1On_;
	bool ch2On_;
	bool extOn_;
	int maxpower_;
	double powerCh1_;
	double powerCh2_;
	double finea_;
	double fineb_;
};

// ------------------------------------
// iBeamSmart laser controller with a 
// single channel (channel 1) and the 
// fine option.
// ------------------------------------

class FineiBeamSmart: public CGenericBase<FineiBeamSmart>
{
public:
    FineiBeamSmart();
    ~FineiBeamSmart();

    // MMDevice API
    int Initialize();
    int Shutdown();

    void GetName(char* pszName) const;
    bool Busy();

	// getters
	int getMaxPower(int* maxpower);
	int getPower(double* power);
	int getChannelStatus(bool* status);
	int getFineStatus(bool* status);
	int getFinePercentage(char fine, double* value);
	int getLaserStatus(bool* status);
	int getSerial(std::string* serial);
	int getFirmwareVersion(std::string* version);
	int getClipStatus(std::string* status);

	// setters
	int setLaserOnOff(bool b);
	int enableChannel(int channel, bool enable);
	int setPower(int channel, double pow);
	int setFineA(double perc);
	int setFineB(double perc);
	int disableExt();
	int enableFine(bool b);
	int setPromptOff();
	int setTalkUsual();

    // action properties
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnLaserOnOff(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableFine(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFineA(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFineB(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnClip(MM::PropertyBase* pProp, MM::ActionType eAct);
	
	// convenience function 
	bool isError(std::string answer);
	bool isOk(std::string answer);
	int getError(std::string error);
	int publishError(std::string error);
	std::string to_string(double x);

private:
	std::string port_;
	std::string serial_;
	std::string clip_;    
	bool initialized_;
	bool busy_;
	bool laserOn_;
	bool fineOn_;
	bool chOn_;
	int maxpower_;
	double powerCh_;
	double finea_;
	double fineb_;
};

// --------------------------------------
// Simplified iBeamSmart laser controller
// with only channel 1 and no option.
// ------------------------------------

class SimpleiBeamSmart: public CGenericBase<SimpleiBeamSmart>
{
public:
    SimpleiBeamSmart();
    ~SimpleiBeamSmart();

    // MMDevice API
    int Initialize();
    int Shutdown();

    void GetName(char* pszName) const;
    bool Busy();

	// getters
	int getMaxPower(int* maxpower);
	int getPower(double* power);
	int getChannelStatus(bool* status);
	int getLaserStatus(bool* status);
	int getSerial(std::string* serial);
	int getFirmwareVersion(std::string* version);
	int getClipStatus(std::string* status);

	// setters
	int setLaserOnOff(bool b);
	int enableChannel(int channel, bool enable);
	int setPower(int channel, double pow);
	int disableExt();
	int disableFine();
	int setPromptOff();
	int setTalkUsual();

    // action properties
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnLaserOnOff(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnClip(MM::PropertyBase* pProp, MM::ActionType eAct);
	
	// convenience function 
	bool isError(std::string answer);
	bool isOk(std::string answer);
	int getError(std::string error);
	int publishError(std::string error);
	std::string to_string(double x);

private:
	std::string port_;
	std::string serial_;
	std::string clip_;    
	bool initialized_;
	bool busy_;
	bool laserOn_;
	bool chOn_;
	int maxpower_;
	double powerCh_;
};


// ------------------------------------
// iBeamSmart laser controller with a 
// single channel (channel 2) and the 
// external trigger option.
// ------------------------------------

class ExtTriggeriBeamSmart: public CGenericBase<ExtTriggeriBeamSmart>
{
public:
    ExtTriggeriBeamSmart();
    ~ExtTriggeriBeamSmart();

    // MMDevice API
    int Initialize();
    int Shutdown();

    void GetName(char* pszName) const;
    bool Busy();

	// getters
	int getMaxPower(int* maxpower);
	int getPower(double* power);
	int getChannelStatus(bool* status);
	int getExtStatus(bool* status);
	int getLaserStatus(bool* status);
	int getSerial(std::string* serial);
	int getFirmwareVersion(std::string* version);
	int getClipStatus(std::string* status);

	// setters
	int setLaserOnOff(bool b);
	int enableChannel(int channel, bool enable);
	int setPower(int channel, double pow);
	int disableFine();
	int enableExt(bool b);
	int setPromptOff();
	int setTalkUsual();

    // action properties
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnLaserOnOff(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableCh(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnEnableExt(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnClip(MM::PropertyBase* pProp, MM::ActionType eAct);
	
	// convenience function 
	bool isError(std::string answer);
	bool isOk(std::string answer);
	int getError(std::string error);
	int publishError(std::string error);
	std::string to_string(double x);

private:
	std::string port_;
	std::string serial_;
	std::string clip_;    
	bool initialized_;
	bool busy_;
	bool laserOn_;
	bool extOn_;
	bool chOn_;
	int maxpower_;
	double powerCh_;
};