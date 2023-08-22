///////////////////////////////////////////////////////////////////////////////
// FILE:          PI_FL.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   PI Focus Lock
//
// AUTHOR:        
// COPYRIGHT:     
// LICENSE:       
//

#ifdef WIN32
#include <windows.h>
#endif
#include "FixSnprintf.h"

#include "PI_FocusLock.h"
#include <string>
#include <math.h>
#include "ModuleInterface.h"
#include <sstream>

const char* g_PI_ZStageDeviceName = "PIZStage";
const char* g_PI_ZStageAxisName = "Axis";
const char* g_PI_ZStageAxisLimitUm = "Limit_um";

const char* g_PropertyWaitForResponse = "WaitForResponse";
const char* g_Yes = "Yes";
const char* g_No = "No";

int g_ExternalSensor = 0;	
int g_Monitoring = 0;																										//////

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_PI_ZStageDeviceName, MM::StageDevice, "PIZStage Focus-lock");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_PI_ZStageDeviceName) == 0)
   {
      PIZStage* s = new PIZStage();
      return s;
   }

   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

// General utility function:
int ClearPort(MM::Device& device, MM::Core& core, std::string port)
{
   // Clear contents of serial port 
   const unsigned int bufSize = 255;
   unsigned char clear[bufSize];                      
   unsigned long read = bufSize;
   int ret;                                                                   
   while (read == bufSize)                                                   
   {                                                                     
      ret = core.ReadFromSerial(&device, port.c_str(), clear, bufSize, read);
      if (ret != DEVICE_OK)                               
         return ret;                                               
   }
   return DEVICE_OK;                                                           
} 
 

///////////////////////////////////////////////////////////////////////////////
// PIZStage

PIZStage::PIZStage() :
   port_("Undefined"),
   stepSizeUm_(0.1),
  // mThread_(0),
   initialized_(false),
   locked_(false),
   answerTimeoutMs_(1000),
   axisLimitUm_(500.0)
{
   InitializeDefaultErrorMessages();

   // create pre-initialization properties
   // ------------------------------------

   // Name
   CreateProperty(MM::g_Keyword_Name, g_PI_ZStageDeviceName, MM::String, true);

   // Description
   CreateProperty(MM::g_Keyword_Description, "Physik Instrumente (PI) Focus Lock", MM::String, true);

   // Port
   CPropertyAction* pAct = new CPropertyAction (this, &PIZStage::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

   // Axis name
   pAct = new CPropertyAction (this, &PIZStage::OnAxisName);
   CreateProperty(g_PI_ZStageAxisName, "Z", MM::String, false, pAct, true);

   // axis limit in um
   pAct = new CPropertyAction (this, &PIZStage::OnAxisLimit);
   CreateProperty(g_PI_ZStageAxisLimitUm, "500.0", MM::Float, false, pAct, true);

}

PIZStage::~PIZStage()
{
   Shutdown();
}

void PIZStage::GetName(char* Name) const
{
   CDeviceUtils::CopyLimitedString(Name, g_PI_ZStageDeviceName);
}

int PIZStage::Initialize()
{
	// test ismoving
	checkIsMoving_ = true;
	Busy();
   // Command level to 1																												//////////////
   int ret = SendSerialCommand(port_.c_str(), "CCL 1 advanced", "\n");
   if (ret != DEVICE_OK)
      return ret; 

   // Set PI sensors to internal
   ret = Set2Internal();
   if (ret != DEVICE_OK)
      return ret;    

   /*																			// Not needed since done in Set2Internal
   // switch on servo, otherwise "MOV" will fail
   ostringstream command;
   command << "SVO " << axisName_<<" 1";
   ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\n");
   if (ret != DEVICE_OK)
      return ret;*/

   CDeviceUtils::SleepMs(10);
   ret = GetPositionSteps(curSteps_);
   if (ret != DEVICE_OK)
      return ret;

   // StepSize
   CPropertyAction* pAct = new CPropertyAction (this, &PIZStage::OnStepSizeUm);
   CreateProperty("StepSizeUm", "0.01", MM::Float, false, pAct);
   stepSizeUm_ = 0.01;   

   // position
   pAct = new CPropertyAction (this, &PIZStage::OnPosition);
   CreateProperty(MM::g_Keyword_Position, "0.0", MM::Float, false, pAct);
   SetPropertyLimits(MM::g_Keyword_Position, 0, axisLimitUm_);
  

  /* // External sensor position
   pAct = new CPropertyAction (this, &PIZStage::OnPosition);
   CreateProperty("", "0.0", MM::Float, false, pAct);
   SetPropertyLimits("", 0, axisLimitUm_);
   */

 /*  // Internal sensor position
   pAct = new CPropertyAction (this, &PIZStage::OnIntSensorPosition);
   CreateProperty("Int sensor pos", "0.0", MM::Float, true, pAct); 
   log << "internal sensor position\n";    
   */

   //////////////////////////////// Focus lock property																								//////
   pAct = new CPropertyAction (this, &PIZStage::OnSensorState);
   CreateProperty("External sensor", "0", MM::Integer, false,pAct);
   AddAllowedValue("External sensor", "1");   
   AddAllowedValue("External sensor", "0");   
   
   //////////////////////////////// launch or stop thread																							//////
 /*  pAct = new CPropertyAction (this, &PIZStage::OnMonitoring);
   CreateProperty("Pos monitor", "0", MM::Integer, false,pAct);
   AddAllowedValue("Pos monitor", "1");   
   AddAllowedValue("Pos monitor", "0");   
   log << "position monitoring\n";
   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;
	  */
   //mThread_ = new PIMonitorThread(*this, true);
   //mThread_->Start();
   initialized_ = true;
   return DEVICE_OK;
}

int PIZStage::Shutdown()
{
   if (initialized_)
   {
	 // delete(mThread_);
	  Set2Internal();																														//////
      initialized_ = false;
   }
   return DEVICE_OK;
}

bool PIZStage::Busy()
{
   if (!checkIsMoving_)
      return false;
   unsigned char c = (unsigned char) 5;
   // send command
   int ret = WriteToComPort(port_.c_str(), &c, 1);
   if (ret != DEVICE_OK){
	   return false;
   }

   // block/wait for acknowledge, or until we time out;
   string answer;
   ret = GetSerialAnswer(port_.c_str(), "\n", answer);
   if (ret != DEVICE_OK)
   {
      // "#5" failed, maybe controller does not support this
      // clear error with two "ERR?"
      GetError();
      GetError();
      checkIsMoving_ = false;
      return false;
   }
   long isMoving;
   if (!GetValue(answer, isMoving)){
      return false;
   }

   return (isMoving != 0);
}

int PIZStage::SetPositionSteps(long steps)
{
   double pos = steps * stepSizeUm_;
   return SetPositionUm(pos);
}

int PIZStage::GetPositionSteps(long& steps)
{
   double pos;
   int ret = GetPositionUm(pos);
   if (ret != DEVICE_OK)
      return ret;
   steps = (long) ((pos / stepSizeUm_) + 0.5);
   return DEVICE_OK;
}
  
int PIZStage::SetPositionUm(double pos)
{
	if(!locked_){
	   ostringstream command;
	   command << "MOV " << axisName_<< " " << pos;

	   // send command
	   int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\n");
	   if (ret != DEVICE_OK){
		   return ret;
	   }
	   CDeviceUtils::SleepMs(20);
	   // block/wait for acknowledge, or until we time out;

		return GetError();
	}

   return DEVICE_OK;
}

int PIZStage::GetError()
{
   int ret = SendSerialCommand(port_.c_str(), "ERR?", "\n");
   if (ret != DEVICE_OK){
      return ret;
   }
   string answer;
   ret = GetSerialAnswer(port_.c_str(), "\n", answer);
   if (ret != DEVICE_OK){
      return ret;
   }

   int errNo = atoi(answer.c_str());
   if (errNo == 0){
	   return DEVICE_OK;
   }
   return ERR_OFFSET + errNo;   
}

int PIZStage::GetPositionUm(double& pos)
{
  
   /*

   ///////////////////////////////////////////POS
   ostringstream command;
   command << "POS? " << axisName_;

   // send command
   int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\n");
   if (ret != DEVICE_OK){
      return ret;
   }

   // block/wait for acknowledge, or until we time out;
   string answer;
   ret = GetSerialAnswer(port_.c_str(), "\n", answer);
   if (ret != DEVICE_OK){
      return ret;
	  }

   if (!GetValue(answer, pos)){
      return ERR_UNRECOGNIZED_ANSWER;
   }
   */
   
   ///////////////////////////////////////////POS
   //double pos_;
   //double& posr = pos_;
   ostringstream command2;
   command2 << "TSP? " << 1;

   // send command
   int ret = SendSerialCommand(port_.c_str(), command2.str().c_str(), "\n");
   if (ret != DEVICE_OK){
	  return ret;
   }

   // block/wait for acknowledge, or until we time out;
   string answer;
   ret = GetSerialAnswer(port_.c_str(), "\n", answer);
   if (ret != DEVICE_OK){
	   return ret;
   }

   if (!GetValue(answer, pos))	{
	  																////////////////////////////////////////// creates error?
      return ERR_UNRECOGNIZED_ANSWER;
   }
   
  /* log << "POS? " << &pos << "\n";
   log << "TSP? " << pos_ << "\n"; 
   log << "reference POS? " << pos << "\n";
   log << "reference TSP? " << posr << "\n";
   */

 return DEVICE_OK;
}

/*
int PIZStage::GetIntSensorPosition(double* pos)															//////////////////////// why * and not &?
{
   std::ofstream log;
   log.open ("Log_PI_Zstage.txt", std::ios::app);
   log << "------- Get internal sensor position\n";
   ostringstream command;
   command << "TSP? " << 1;
   log << "Send command\n";
   // send command
   int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\n");
   if (ret != DEVICE_OK){
	   log << "Send problem\n";
      return ret;
   }

   // block/wait for acknowledge, or until we time out;
   string answer;
   log << "Get answer\n";
   ret = GetSerialAnswer(port_.c_str(), "\n", answer);
   if (ret != DEVICE_OK){
	   log << "Answer problem\n";
      return ret;
   }

   if (!GetValue(answer, *pos))	{
	   log << "Unrecognize answer\n";																	////////////////////////////////////////// creates error?
      return ERR_UNRECOGNIZED_ANSWER;
   }
    log.close();
   return DEVICE_OK;
}
*/
int PIZStage::SetOrigin()
{
   return DEVICE_UNSUPPORTED_COMMAND;
}

int PIZStage::GetLimits(double& min, double& max)
{
   return DEVICE_UNSUPPORTED_COMMAND;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PIZStage::Set2External()					// Set to external sensors
{				
	string answer;							

	SetServoState(0);
	double low,high;

	// Set to external sensor
	int ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000500 0", "\n");
	if (ret != DEVICE_OK)
		return ret;
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000501 1", "\n");
	if (ret != DEVICE_OK) 
		return ret;

	// Set servo parameters P,I
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000300 0.02", "\n");
	if (ret != DEVICE_OK)
		return ret;		
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000301 2e-3", "\n");
	if (ret != DEVICE_OK)
		return ret;	

	// Set soft limits
	ret=SendSerialCommand(port_.c_str(), "VOL? 1", "\n");
	if (ret != DEVICE_OK)
		return ret;
	ret = GetSerialAnswer(port_.c_str(), "\n", answer);
	if (ret != DEVICE_OK)
		return ret;
		
	answer=answer.substr(2,answer.length()-3);	
	
	low=atof(answer.c_str())-25;																											// Should come back to that: soft limits not fixed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	high=low+50;																															/////////////////////////////////////////////////////////////
	//low = 80;
	//high = 110; 

	ostringstream command1,command2;
	command1 << "SPA 1 0x0c000000 " << low;
	command2 << "SPA 1 0x0c000001 " << high;
	ret=SendSerialCommand(port_.c_str(), command1.str().c_str(), "\n");
	if (ret != DEVICE_OK)
		return ret;				
	ret=SendSerialCommand(port_.c_str(), command2.str().c_str(), "\n");
	if (ret != DEVICE_OK)
		return ret;
					
	SetServoState(1);

	
//	log << "set soft limit\n";
	// Move to position 0
	SetPositionUm(0);															
	
	locked_ = true;

	return DEVICE_OK;
}

int PIZStage::Set2Internal()				// Set sensors to internal
{
												
	SetServoState(0);
				
	// Set to internal sensor			
	int ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000500 1", "\n");
	if (ret != DEVICE_OK)
		return ret;
	
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000501 0", "\n");
	if (ret != DEVICE_OK)
		return ret;
		
	// Set servo parameters
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000300 0.02", "\n");
	if (ret != DEVICE_OK)
		return ret;
			
	ret=SendSerialCommand(port_.c_str(), "SPA Z 0x07000301 1.567286e-4", "\n");
	if (ret != DEVICE_OK)
		return ret;
	// Set soft limits
	ret=SendSerialCommand(port_.c_str(), "SPA 1 0x0c000000 -30", "\n");
	if (ret != DEVICE_OK)
		return ret;
	
	ret=SendSerialCommand(port_.c_str(), "SPA 1 0x0c000001 130", "\n");
	if (ret != DEVICE_OK)
		return ret;
	SetServoState(1);

	locked_ = false;

	return DEVICE_OK;
}

int PIZStage::SetServoState(int state)
{
    ostringstream command;
    command << "SVO " << axisName_<< " " << state;

    // send command
    int ret = SendSerialCommand(port_.c_str(), command.str().c_str(), "\n");
    if (ret != DEVICE_OK)
      return ret;

    CDeviceUtils::SleepMs(20);
	return DEVICE_OK;
}

int PIZStage::ReportStateChange(double newState)
{
   std::ostringstream os;
   os << newState;
   return OnPropertyChanged("Int sensor pos", os.str().c_str());
}
/*
int PIZStage::StartThread(){
	std::ofstream log;
	log.open ("Log_PI_Zstage.txt", ios::app);
    log << "------- Start thread\n";
    mThread_->Start();
	log.close();
	return DEVICE_OK;
}

int PIZStage::StopThread(){
	std::ofstream log;
	log.open ("Log_PI_Zstage.txt", ios::app);
    log << "------- Start thread\n";
    mThread_->Stop();
	log.close();
	return DEVICE_OK;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////

/*
int PIZStage::OnIntSensorPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      double pos;
      int ret = GetIntSensorPosition(&pos);
      if (ret != DEVICE_OK)
         return ret;

      pProp->Set(pos);
   }
   return DEVICE_OK;
}*/
/*
int PIZStage::OnMonitoring(MM::PropertyBase* pProp, MM::ActionType eAct)																						/////
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long)g_Monitoring);
   }
   else if (eAct == MM::AfterSet)
   {
	  long pos;
      pProp->Get(pos);
     
	  if(pos==1)
	  {
		 StartThread();
	  }
	  else if(pos==0)
	  {
		 StopThread();
	  }
	  g_Monitoring = pos;
   }

   return DEVICE_OK;
}*/

int PIZStage::OnSensorState(MM::PropertyBase* pProp, MM::ActionType eAct)																						/////
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long)g_ExternalSensor);
   }
   else if (eAct == MM::AfterSet)
   {
	  long pos;
      pProp->Get(pos);
     
	  if(pos==1)
	  {
		  Set2External();
	  }
	  else if(pos==0)
	  {
		  Set2Internal();
	  }
	  g_ExternalSensor = pos;
   }

   return DEVICE_OK;
}

int PIZStage::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
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

int PIZStage::OnAxisName(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(axisName_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(axisName_);
   }

   return DEVICE_OK;
}

int PIZStage::OnStepSizeUm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(stepSizeUm_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(stepSizeUm_);
   }

   return DEVICE_OK;
}

int PIZStage::OnAxisLimit(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(axisLimitUm_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(axisLimitUm_);
   }

   return DEVICE_OK;
}

int PIZStage::OnPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      double pos;
      int ret = GetPositionUm(pos);
      if (ret != DEVICE_OK)
         return ret;

      pProp->Set(pos);
   }
   else if (eAct == MM::AfterSet)
   {
      double pos;
      pProp->Get(pos);
      int ret = SetPositionUm(pos);
      if (ret != DEVICE_OK)
         return ret;

   }

   return DEVICE_OK;
}


bool PIZStage::GetValue(string& sMessage, double& dval)
{
   if (!ExtractValue(sMessage))
      return false;
   
   char *pend;
   const char* szMessage = sMessage.c_str();
   double dValue = strtod(szMessage, &pend);
   
   // return true only if scan was stopped by spaces, linefeed or the terminating NUL and if the
   // string was not empty to start with
   if (pend != szMessage)
   {
      while( *pend!='\0' && (*pend==' '||*pend=='\n')) pend++;
      if (*pend=='\0')
      {
         dval = dValue;
         return true;
      }
   }
   return false;
}

bool PIZStage::GetValue(string& sMessage, long& lval)
{
   if (!ExtractValue(sMessage))
      return false;

   char *pend;
   const char* szMessage = sMessage.c_str();
   long lValue = strtol(szMessage, &pend, 0);
   
   // return true only if scan was stopped by spaces, linefeed or the terminating NUL and if the
   // string was not empty to start with
   if (pend != szMessage)
   {
      while( *pend!='\0' && (*pend==' '||*pend=='\n')) pend++;
      if (*pend=='\0')
      {
         lval = lValue;
         return true;
      }
   }
   return false;
}

bool PIZStage::ExtractValue(std::string& sMessage)
{
   // value is after last '=', if any '=' is found
   size_t p = sMessage.find_last_of('=');
   if ( p != std::string::npos )
       sMessage.erase(0,p+1);
   
   // trim whitspaces from right ...
   p = sMessage.find_last_not_of(" \t\r\n");
   if (p != std::string::npos)
       sMessage.erase(++p);
   
   // ... and left
   p = sMessage.find_first_not_of(" \n\t\r");
   if (p == std::string::npos)
      return false;
   
   sMessage.erase(0,p);
   return true;
}

bool PIZStage::waitForResponse()
{
   char val[MM::MaxStrLength];
   int ret = GetProperty(g_PropertyWaitForResponse, val);
   assert(ret == DEVICE_OK);
   if (strcmp(val, g_Yes) == 0)
      return true;
   else
      return false;
}


/*
////////// Thread monitor
PIMonitorThread::PIMonitorThread(PIZStage& PI, bool debug) :
   state_(0),
   PI_(PI),
   debug_(debug)
{
};

PIMonitorThread::~PIMonitorThread()
{
   Stop();
   wait();
}

int PIMonitorThread::svc() 
{
   std::ofstream log;
   log.open ("Log_PI_thread.txt", ios::app);
   log << "------- Loop\n";
   while (!stop_)
   {/*
      long state;
      int ret = aInput_.GetDigitalInput(&state);
      if (ret != DEVICE_OK)
      {
         stop_ = true;
         return ret;
      }

      if (state != state_) 
      {
         aInput_.ReportStateChange(state);
         state_ = state;
      }*//*
	  log << "Get internal sensor position\n";
	  double state;
      int ret = PI_.GetIntSensorPosition(&state);
	  if (ret != DEVICE_OK)
      {
         log << "Answer problem\n";
         stop_ = true;
		 log << "Stopping\n";
		 return ret;
      }
	  if (state != state_) 
      {
         log << "State changed\n";
         PI_.ReportStateChange(state);
         state_ = state;
      }
	  log << "Sleep\n";
      CDeviceUtils::SleepMs(750);
   }
   log << "Done\n";
   log.close();
   return DEVICE_OK;
}


void PIMonitorThread::Start()
{
   std::ofstream log;
   log.open ("Log_PI_thread.txt", ios::app);
   log << "------- Start thread\n";
   stop_ = false;
   activate();
   log.close();
}

*/