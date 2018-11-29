//-----------------------------------------------------------------------------
// FILE:          Thorlabs_ElliptecSlider.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Controls the Elliptec 4-position sliders from Thorlabs
// COPYRIGHT:     EMBL
// LICENSE:       LGPL
// AUTHOR:        Joran Deschamps and Anindita Dasgupta, 2018
//-----------------------------------------------------------------------------

#include "ThorlabsElliptecSlider.h"
#include <iostream>
#include <fstream>

const char* g_DeviceName = "Thorlabs ElliptecSlider";
const char* g_pos0 = "00000000"; // The positions were experimentally determined
const char* g_pos1 = "0000001F";
const char* g_pos2 = "0000003E";
const char* g_pos3 = "0000005D";

//-----------------------------------------------------------------------------
// MMDevice API
//-----------------------------------------------------------------------------

MODULE_API void InitializeModuleData()
{
	RegisterDevice(g_DeviceName, MM::StateDevice, g_DeviceName);
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
	if (deviceName == 0)
		return 0;

	if (strcmp(deviceName, g_DeviceName) == 0)
	{
		return new ThorlabsElliptecSlider();
	}

	return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}


//-----------------------------------------------------------------------------
// ThorlabsElliptecSlider device adapter
//-----------------------------------------------------------------------------

ThorlabsElliptecSlider::ThorlabsElliptecSlider():
	port_("Undefined"),
	numPos_(4),
	channel_("0"),
	initialized_(false),
	busy_(false)
{
	InitializeDefaultErrorMessages();
	
	SetErrorText(ERR_PORT_CHANGE_FORBIDDEN, "Port change is forbidden.");
	SetErrorText(ERR_UNEXPECTED_ANSWER, "The device returned an unexpected answer.");
	SetErrorText(ERR_WRONG_DEVICE, "The device is not an ELL9.");
	SetErrorText(ERR_FORBIDDEN_POSITION_REQUESTED, "Forbidden position requested (allowed: 0, 1, 2 and 3).");
	SetErrorText(ERR_UNKNOWN_STATE, "Unknown state.");
	
	SetErrorText(ERR_COMMUNICATION_TIME_OUT, "Communication time-out.");
	SetErrorText(ERR_MECHANICAL_TIME_OUT, "Mechanical time-out.");
	SetErrorText(ERR_COMMAND_ERROR_OR_NOT_SUPPORTED, "Unsupported or unknown command.");
	SetErrorText(ERR_VALUE_OUT_OF_RANGE, "Value out of range.");
	SetErrorText(ERR_MODULE_ISOLATED, "Module isolated.");
	SetErrorText(ERR_MODULE_OUT_OF_ISOLATION, "Module out of isolation.");
	SetErrorText(ERR_INITIALIZING_ERROR, "Initializing error.");
	SetErrorText(ERR_THERMAL_ERROR, "Theromal error.");
	SetErrorText(ERR_BUSY, "Busy.");
	SetErrorText(ERR_SENSOR_ERROR, "Sensor error.");
	SetErrorText(ERR_MOTOR_ERROR, "Motor error.");
	SetErrorText(ERR_OUT_OF_RANGE, "Out of range.");
	SetErrorText(ERR_OVER_CURRENT_ERROR, "Over-current error.");
	SetErrorText(ERR_UNKNOWN_ERROR, "Unknown error (error code >13).");
	
	// Description
	CreateProperty(MM::g_Keyword_Description, "Thorlabs Elliptec 4-position Slider controller", MM::String, true);

	// Port
	CPropertyAction* pAct = new CPropertyAction (this, &ThorlabsElliptecSlider::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

	// Channel
	std::string channels[] = {"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
	std::vector<std::string> channels_vec;
	for(int i=0;i<16;i++){
		channels_vec.push_back(channels[i]);
	}

	pAct = new CPropertyAction (this, &ThorlabsElliptecSlider::OnChannel);
	CreateProperty("Channel", "0", MM::String, false, pAct, true);
	SetAllowedValues("Channel", channels_vec);

}

ThorlabsElliptecSlider::~ThorlabsElliptecSlider()
{
	Shutdown();
}

void ThorlabsElliptecSlider::GetName(char* Name) const
{
	CDeviceUtils::CopyLimitedString(Name, g_DeviceName);
}

int ThorlabsElliptecSlider::Initialize()
{
	// ID
	std::string id;
	getID(&id);
	int nRet = CreateProperty("ID", id.c_str(), MM::String, true);
	if (nRet != DEVICE_OK)
		return nRet;

	// State
	CPropertyAction* pAct = new CPropertyAction(this, &ThorlabsElliptecSlider::OnState);
	nRet = CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
	if (nRet != DEVICE_OK)
		return nRet;

	// Label
	pAct = new CPropertyAction (this, &CStateBase::OnLabel);
	nRet = CreateProperty(MM::g_Keyword_Label, "Undefined", MM::String, false, pAct);
	if (nRet != DEVICE_OK)
		return nRet;

	for(int i=0; i<numPos_; i++){
		std::stringstream ss;
		ss << i;
		AddAllowedValue(MM::g_Keyword_State, ss.str().c_str());

		std::ostringstream label;
		label << "Position " << i;
		SetPositionLabel(i,label.str().c_str());
	}

	initialized_ = true;
	return DEVICE_OK;
}

int ThorlabsElliptecSlider::Shutdown()
{
	if (initialized_){
		initialized_ = false;	 
	}
	return DEVICE_OK;
}

bool ThorlabsElliptecSlider::Busy(){
	std::ostringstream command;
	command << channel_ << "gs";

	int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\r");
	if (ret != DEVICE_OK) 
		return true;

	std::string answer;
	ret = GetSerialAnswer(port_.c_str(), "\r", answer);
	if (ret != DEVICE_OK)
		return true;

	int code = getErrorCode(answer.substr(4,answer.length()-4));

	if(code == 0){
		return false;
	} 

	return true;
}

//---------------------------------------------------------------------------
// Setters
//---------------------------------------------------------------------------
int ThorlabsElliptecSlider::setState(int state){
	
	std::ostringstream command;
	command << channel_ << "ma";
	
	std::string pos;
	switch(state){
	case 0:
		pos = g_pos0;
		break;
	case 1:
		pos = g_pos1;
		break;
	case 2:
		pos = g_pos2;
		break;
	case 3:
		pos = g_pos3;
		break;
	default:
		return ERR_FORBIDDEN_POSITION_REQUESTED;
	}

	command << pos;
	int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\r");
	if (ret != DEVICE_OK) 
		return ret;

	std::string answer;
	ret = GetSerialAnswer(port_.c_str(), "\r", answer);
	if (ret != DEVICE_OK)
		return ret;

	// check for error
	if(isError(answer)){
		return getErrorCode(answer.substr(4,answer.length()-4));
	}

	return DEVICE_OK;
}

//---------------------------------------------------------------------------
// Getters
//---------------------------------------------------------------------------
int ThorlabsElliptecSlider::getID(std::string* id){
	std::ostringstream command;
	command << channel_ << "in";

	int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\r");
	if (ret != DEVICE_OK) 
		return ret;

	std::string answer;
	ret = GetSerialAnswer(port_.c_str(), "\r", answer);
	if (ret != DEVICE_OK)
		return ret;

	if(answer.substr(1,2).compare("IN") != 0)
		return ERR_UNEXPECTED_ANSWER;

	if(answer.substr(3,2).compare("09") != 0)
		return ERR_WRONG_DEVICE;

	*id = answer.substr(3,15);

	return DEVICE_OK;
}


int ThorlabsElliptecSlider::getState(int* state){
	std::ostringstream command;
	command << channel_ << "gp";

	int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\r");
	if (ret != DEVICE_OK) 
		return ret;

	std::string answer;
	ret = GetSerialAnswer(port_.c_str(), "\r", answer);
	if (ret != DEVICE_OK)
		return ret;

	// check for error
	if(isError(answer)){
		return getErrorCode(answer.substr(4,answer.length()-4));
	}

	if(answer.substr(2,2).compare("PO") != 0)
		return ERR_UNEXPECTED_ANSWER;

	if(answer.substr(4,answer.length()-4).compare(g_pos0) == 0){
		*state = 0;
	} else if(answer.substr(4,answer.length()-4).compare(g_pos1) == 0){
		*state = 1;
	} else if(answer.substr(4,answer.length()-4).compare(g_pos2) == 0){
		*state = 2;
	} else if(answer.substr(4,answer.length()-4).compare(g_pos3) == 0){
		*state = 3;
	} else {
		return ERR_UNKNOWN_STATE;
	}


	return DEVICE_OK;
}


//---------------------------------------------------------------------------
// Convenience function
//---------------------------------------------------------------------------
bool ThorlabsElliptecSlider::isError(std::string message){
	if(message.substr(2,2).compare("GS") == 0){
		return true;
	}
	return false;
}

int ThorlabsElliptecSlider::getErrorCode(std::string message){
	if(message.compare("00") == 0){
		return DEVICE_OK;
	} else if(message.compare("01") == 0){
		return ERR_COMMUNICATION_TIME_OUT;
	} else if(message.compare("02") == 0){
		return ERR_MECHANICAL_TIME_OUT;
	} else if(message.compare("03") == 0){
		return ERR_COMMAND_ERROR_OR_NOT_SUPPORTED;
	} else if(message.compare("04") == 0){
		return ERR_VALUE_OUT_OF_RANGE;
	} else if(message.compare("05") == 0){
		return ERR_MODULE_ISOLATED;
	} else if(message.compare("06") == 0){
		return ERR_MODULE_OUT_OF_ISOLATION;
	} else if(message.compare("07") == 0){
		return ERR_INITIALIZING_ERROR;
	} else if(message.compare("08") == 0){
		return ERR_THERMAL_ERROR;
	} else if(message.compare("09") == 0){
		return ERR_BUSY;
	} else if(message.compare("0A") == 0){
		return ERR_SENSOR_ERROR;
	} else if(message.compare("0B") == 0){
		return ERR_MOTOR_ERROR;
	} else if(message.compare("0C") == 0){
		return ERR_OUT_OF_RANGE;
	} else if(message.compare("0D") == 0){
		return ERR_OVER_CURRENT_ERROR;
	}

	return ERR_UNKNOWN_ERROR;
}

//---------------------------------------------------------------------------
// Action handlers
//---------------------------------------------------------------------------

int ThorlabsElliptecSlider::OnState(MM::PropertyBase* pProp, MM::ActionType eAct){	
	if (eAct == MM::BeforeGet){ 
		int state;
		int ret = getState(&state);
		if(ret != DEVICE_OK)
			return ret;
		
		std::stringstream ss;
		ss << state;

		pProp->Set(ss.str().c_str());

	} else if (eAct == MM::AfterSet){
		long state;
		pProp->Get(state);
		int ret = setState(state);
		if(ret != DEVICE_OK)
			return ret;
	}

	return DEVICE_OK;
}

int ThorlabsElliptecSlider::OnPort(MM::PropertyBase* pProp , MM::ActionType eAct)
{
	if (eAct == MM::BeforeGet)
	{
		pProp->Set(port_.c_str());
	}
	else if (eAct == MM::AfterSet)
	{
		if (initialized_)
		{
			// revert
			pProp->Set(port_.c_str());
			return ERR_PORT_CHANGE_FORBIDDEN;
		}

		pProp->Get(port_);
	}

	return DEVICE_OK;
}

int ThorlabsElliptecSlider::OnChannel(MM::PropertyBase* pProp , MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		std::string channel;
		pProp->Get(channel);

		channel_ = channel;
	}

	return DEVICE_OK;
}

