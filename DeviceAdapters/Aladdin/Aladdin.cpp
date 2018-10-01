///////////////////////////////////////////////////////////////////////////////
// FILE:          Aladdin.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Aladdin pump controller adapter
// COPYRIGHT:     University of California, San Francisco, 2011
//
// AUTHOR:        Original device adapter by Kurt Thorn, UCSF, November 2011
//				  Extension to multi-pumps and the possibility to set custom
//                programs on each pump by Joran Deschamps and Thomas Chartier,
//				  EMBL, May 2016	
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// CVS:           
//



#ifdef WIN32
   #include <windows.h>
   #define snprintf _snprintf 
#endif


#include "../../MMDevice/MMDevice.h"
#include "Aladdin.h"
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMDevice/DeviceUtils.h"
#include <iostream>
#include <fstream>


// Controller
const char* g_ControllerName = "Aladdin";
const char* g_Keyword_Infuse = "Infuse";
const char * g_Keyword_Withdraw = "Withdraw";
const char * carriage_return = "\r";
const char * line_feed = "\n";



///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_ControllerName, MM::GenericDevice, "Aladdin Syringe Pump");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_ControllerName) == 0)
   {
      // create AladdinController
      AladdinController* pController = new AladdinController(g_ControllerName);
      return pController;
   }

   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// AladdinController implementation
// ~~~~~~~~~~~~~~~~~~~~

AladdinController::AladdinController(const char* name) :
   initialized_(false), 
   name_(name), 
   error_(0),
   changedTime_(0.0)
{
   assert(strlen(name) < (unsigned int) MM::MaxStrLength);

   InitializeDefaultErrorMessages();

   // create pre-initialization properties
   // ------------------------------------

   // Name
   CreateProperty(MM::g_Keyword_Name, name_.c_str(), MM::String, true);

   // Description
   CreateProperty(MM::g_Keyword_Description, "Aladdin Syringe Pump", MM::String, true);

   // Port
   CPropertyAction* pAct = new CPropertyAction (this, &AladdinController::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

   // Number of pumps
   pAct = new CPropertyAction(this, &AladdinController::OnPumpNumber);
   CreateProperty("PumpNr", "1", MM::Integer, false, pAct, true);
   SetPropertyLimits("PumpNr", 1, 99);

   EnableDelay(); // signals that the delay setting will be used
   UpdateStatus();
}

AladdinController::~AladdinController()
{
   Shutdown();
}

bool AladdinController::Busy()
{
   MM::MMTime interval = GetCurrentMMTime() - changedTime_;
   MM::MMTime delay(GetDelayMs()*1000.0);
   if (interval < delay)
      return true;
   else
      return false;
}

void AladdinController::GetName(char* name) const
{
   assert(name_.length() < CDeviceUtils::GetMaxStringLength());
   CDeviceUtils::CopyLimitedString(name, name_.c_str());
}


int AladdinController::Initialize()
{
   this->LogMessage("AladdinController::Initialize()");
   int ret = 0;

   // for each pump
   for (long i=0; i <Npumps_; i++){
	  stringstream ss;
		ss << "Pump nmbr: "<< i;
	   this->LogMessage(ss.str());
	   ss.clear();

      CPropertyActionEx *pExAct1 = new CPropertyActionEx(this, &AladdinController::OnVolume, i);
      CPropertyActionEx *pExAct2 = new CPropertyActionEx(this, &AladdinController::OnRun, i);
      CPropertyActionEx *pExAct3 = new CPropertyActionEx(this, &AladdinController::OnRate, i);
      CPropertyActionEx *pExAct4 = new CPropertyActionEx(this, &AladdinController::OnDiameter, i);
      CPropertyActionEx *pExAct5 = new CPropertyActionEx(this, &AladdinController::OnPhase, i);
      CPropertyActionEx *pExAct6 = new CPropertyActionEx(this, &AladdinController::OnFunction, i);
      CPropertyActionEx *pExAct7 = new CPropertyActionEx(this, &AladdinController::OnDirection, i);

      std::ostringstream vol,rate,run,diam,phase,func,dir,rateun;
      vol << "Volume (uL) Pump" << i;
      rate << "Rate (uL/min) Pump" << i;
      run << "Run Pump" << i;
      diam << "Diameter (mm) Pump" << i;
      phase << "Phase Pump" << i;
      func << "Function Pump" << i;
      dir << "Direction Pump" << i;
      rateun << "Rate Unit Pump" << i;

	  
	  this->LogMessage("Start creating the properties");
	  
      ret += CreateProperty(vol.str().c_str(), "0.0", MM::Float, false, pExAct1);
      
	  ret += CreateProperty(run.str().c_str(), "0", MM::Integer, false, pExAct2);
      AddAllowedValue(run.str().c_str(), "0");
      AddAllowedValue(run.str().c_str(), "1");
      
	  ret += CreateProperty(rate.str().c_str(), "0.0", MM::Float, false, pExAct3);

	  ret += CreateProperty(diam.str().c_str(), "0.0", MM::Float, false, pExAct4);
      
	  ret += CreateProperty(phase.str().c_str(), "0", MM::Integer, false, pExAct5);
	  SetPropertyLimits(phase.str().c_str(), 1, 41);

      ret += CreateProperty(func.str().c_str(), "", MM::String, false, pExAct6);
      
	  ret += CreateProperty(dir.str().c_str(), "Infuse", MM::String, false, pExAct7);
      AddAllowedValue(dir.str().c_str(), g_Keyword_Infuse);
      AddAllowedValue(dir.str().c_str(), g_Keyword_Withdraw);

	  
	  ss << "Done, ret: "<< ret;
	  this->LogMessage(ss.str());
	  

      if (ret != DEVICE_OK)
         return ret;
   }

   initialized_ = true;
   return HandleErrors();
}

int AladdinController::Shutdown()
{
   if (initialized_)
   {
      initialized_ = false;
   }
   return HandleErrors();
}



///////////////////////////////////////////////////////////////////////////////
// String utilities
///////////////////////////////////////////////////////////////////////////////


void AladdinController::StripString(string& StringToModify)
{
   if(StringToModify.empty()) return;

   size_t startIndex = StringToModify.find_first_not_of(" ");
   size_t endIndex = StringToModify.find_last_not_of(" ");
   string tempString = StringToModify;
   StringToModify.erase();

   StringToModify = tempString.substr(startIndex, (endIndex-startIndex+ 1) );
}



///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////


int AladdinController::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
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

   return HandleErrors();
}

int AladdinController::OnPumpNumber(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(Npumps_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(Npumps_);
   }

   return DEVICE_OK;
}

int AladdinController::OnVolume(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   double volume;
   if (eAct == MM::BeforeGet)
   {
      GetVolume(pump, volume);
      pProp->Set(volume);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(volume);
      SetVolume(pump, volume);
   }   

   return HandleErrors();
}

int AladdinController::OnDiameter(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   double diameter;
   if (eAct == MM::BeforeGet)
   {
      GetDiameter(pump, diameter);
      pProp->Set(diameter);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(diameter);
      SetDiameter(pump, diameter);
   }   

   return HandleErrors();
}

int AladdinController::OnRate(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   double rate;
   if (eAct == MM::BeforeGet)
   {
      GetRate(pump, rate);
      pProp->Set(rate);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(rate);
      SetRate(pump, rate);
   }   

   return HandleErrors();
}

int AladdinController::OnDirection(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{

   string direction;
   if (eAct == MM::BeforeGet)
   {
      GetDirection(pump, direction);
      pProp->Set(direction.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(direction);
      SetDirection(pump, direction);
   }   

   return HandleErrors();
}

int AladdinController::OnRun(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   long run;
   if (eAct == MM::BeforeGet)
   {
      GetRun(pump, run);
      pProp->Set(run);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(run);
      SetRun(pump, run);
   }   

   return HandleErrors();
}

int AladdinController::OnPhase(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   long phase;
   if (eAct == MM::BeforeGet)
   {
      GetPhase(pump, phase);
      pProp->Set(phase);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(phase);
      SetPhase(pump, phase);
   }   

   return HandleErrors();
}

int AladdinController::OnFunction(MM::PropertyBase* pProp, MM::ActionType eAct, long pump)
{
   string function;
   if (eAct == MM::BeforeGet)
   {
      GetFunction(pump, function);
	  pProp->Set(function.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(function);
      SetFunction(pump, function);
   }   

   return HandleErrors();
}

///////////////////////////////////////////////////////////////////////////////
// Utility methods
///////////////////////////////////////////////////////////////////////////////

/*void AladdinController::CreateDefaultProgram()
//Set up the default program that we can run to do a simple infusion
{

   //test 2
   SendPurge("0 PHN1");
   SendPurge("0 FUN RAT");
   SendPurge("0 RAT 300 UM");
   SendPurge("0 VOL 20");
   SendPurge("0 PHN2");
   SendPurge("0 FUN INC");
   SendPurge("0 RAT 300");
   SendPurge("0 VOL 50");
   SendPurge("0 PHN3");
   SendPurge("0 FUN STP");

   SendPurge("1 PHN1");
   SendPurge("1 FUN RAT");
   SendPurge("1 RAT 380 UM");
   SendPurge("1 VOL 25");
   SendPurge("1 PHN2");
   SendPurge("1 FUN INC");
   SendPurge("1 RAT 380");
   SendPurge("1 VOL 50");
   SendPurge("1 PHN3");
   SendPurge("1 FUN STP");

   SendPurge("2 PHN1");
   SendPurge("2 FUN RAT");
   SendPurge("2 RAT 450 UM");
   SendPurge("2 VOL 30");
   SendPurge("2 PHN2");
   SendPurge("2 FUN INC");
   SendPurge("2 RAT 450");
   SendPurge("2 VOL 50");
   SendPurge("2 PHN3");
   SendPurge("2 FUN STP");

   
   SendPurge("0 PHN1");
   SendPurge("1 PHN1");
   SendPurge("2 PHN1");

}*/

void AladdinController::SendPurge(string s){
   Send(s); //Phase 2
   ReceiveOneLine();
   Purge();
}

void AladdinController::SetVolume(long pump, double volume)
{
   //volume = volume/1000; //pump volume is always set in mL
   stringstream msg;
   msg << pump << "VOL" << volume;
   Purge();
   Send(msg.str());
   ReceiveOneLine();
}

void AladdinController::GetVolume(long pump, double& volume)
{
   stringstream msg;
   string ans;
   string units;
   msg << pump << "0 VOL";
   Purge();
   Send(msg.str());
   buf_string_ = "";
   GetSerialAnswer(port_.c_str(), "L", buf_string_); //Volume string ends in UL or ML

   if (! buf_string_.empty())
       {
         volume = atof(buf_string_.substr(4,5).c_str());
		 units = buf_string_.substr(10,1).c_str();
		 if (units.compare("M") == 0)
			 volume = volume*1000; //return volume in uL
      }

}

void AladdinController::SetDiameter(long pump, double diameter)
{
   stringstream msg;
   msg << pump << "DIA" << diameter;
   Purge();
   Send(msg.str());
   ReceiveOneLine();
}

void AladdinController::GetDiameter(long pump, double& diameter)
{
   stringstream msg;
   msg << pump << "DIA";
   Purge();
   Send(msg.str());
   string answer = "";
   GetUnterminatedSerialAnswer(answer, 10); //awaiting 10 response characters
   if (! answer.empty())
       {
         diameter = atof(answer.substr(4,5).c_str());
      }
}

void AladdinController::SetRate(long pump, double rate)
{
   stringstream msg;
   msg << pump << "RAT" << rate <<"UM"; //Always set rate in uL/min
   Purge();
   Send(msg.str());
   ReceiveOneLine();
}

void AladdinController::GetRate(long pump, double& rate)
{
   stringstream msg;
   msg << pump << "RAT";
   Purge();
   Send(msg.str());
   string answer = "";
   string units;
   GetUnterminatedSerialAnswer(answer, 11); //awaiting 9 response characters
   if (! answer.empty())
       {
         rate = atof(answer.substr(4,5).c_str());
		 units = answer.substr(10,2).c_str();
      }
   //units is UM uL/min, UH uL/hr, MM mL/min, MH, mL/hr
   //we want to return it in uL/min
   if (units.compare("UH") == 0)
			 rate = rate/60; 
   if (units.compare("MH") == 0)
			 rate = (rate*1000)/60;
   if (units.compare("MM") == 0)
			 rate = rate*1000;
}

void AladdinController::SetDirection(long pump, string direction)
{
   stringstream msg;
   if (direction.compare(g_Keyword_Infuse) == 0)
   {
	   msg << pump << "DIR INF";
	   Purge();
	   Send(msg.str());
	   ReceiveOneLine();
   }
   else if (direction.compare(g_Keyword_Withdraw) == 0)
   {
	   msg << pump << "DIR WDR";
	   Purge();
	   Send(msg.str());
	   ReceiveOneLine();
   }

}

void AladdinController::GetDirection(long pump, string& direction)
{
   stringstream msg;
   msg << pump << "DIR";
   Purge();
   Send(msg.str());
   string answer = "";
   GetUnterminatedSerialAnswer(answer, 7); //awaiting 9 response characters
   if (! answer.empty())
       {
         if (answer.substr(4,3).compare("INF") == 0)
			 direction = g_Keyword_Infuse;
		 else if (answer.substr(4,3).compare("WDR") == 0)
			 direction = g_Keyword_Withdraw;
      }
}

void AladdinController::GetRun(long pump, long& run)
{
   stringstream msg;
   msg << pump; 
   Purge();
   Send(msg.str());
   string answer = "";  
   string status = "";
   GetUnterminatedSerialAnswer(answer, 5); //awaiting 9 response characters
   if (! answer.empty())
       {
         if(answer.substr(4,1).compare("I") == 0 || answer.substr(4,1).compare("W") == 0)
			run = 1;
		 else run = 0;
      }
}

void AladdinController::SetRun(long pump, long run)
{
   stringstream msg;
   if (run == 0) //Stop pumping
   {
     msg << pump << "STP";
     Purge();
     Send(msg.str());
     ReceiveOneLine();
   }
   else if (run == 1) //Start pumping
   {
     msg << pump << "RUN";
     Purge();
     Send(msg.str());
     ReceiveOneLine();
   }
}

bool AladdinController::isValidFunction(string function){

	// rewrite with std::find and a vector of strings
	if(function.substr(0,3).compare("RAT") || function.substr(0,3).compare("INC") || function.substr(0,3).compare("DEC") || function.substr(0,3).compare("STP") || function.substr(0,3).compare("JMP") ||
		function.substr(0,3).compare("PRI") || function.substr(0,3).compare("PRL") || function.substr(0,3).compare("LOP") || function.substr(0,3).compare("LPS") || function.substr(0,3).compare("LPE") ||
		 function.substr(0,3).compare("PAS") || function.substr(0,2).compare("IF") || function.substr(0,3).compare("EVN") || function.substr(0,3).compare("EVS") || function.substr(0,3).compare("EVR") || 
		 function.substr(0,3).compare("BEP") || function.substr(0,3).compare("OUT")){
		return true;
	}
	return false;
}
void AladdinController::SetFunction(long pump, string function)
{
	if(isValidFunction(function)){
	 stringstream msg;
	 msg << pump << "FUN" << function;				
	 Purge();
	 Send(msg.str());
	 ReceiveOneLine();
   }
}

void AladdinController::GetFunction(long pump, string& function)
{
   stringstream msg;
   msg << pump << "FUN";
   Purge();
   Send(msg.str());
   string answer = "";
   GetUnterminatedSerialAnswer(answer, 10); //awaiting 10 response characters
   if (! answer.empty()){
         function = answer.substr(4,3).c_str();
   }
}

void AladdinController::SetPhase(long pump, long phase)
{
   stringstream msg;
   msg << pump << "PHN" << phase;				
   Purge();
   Send(msg.str());
   ReceiveOneLine();
}

void AladdinController::GetPhase(long pump, long& phase)
{
   stringstream msg;
   msg << pump << "PHN";
   Purge();
   Send(msg.str());
   string answer = "";
   GetUnterminatedSerialAnswer(answer, 10); //awaiting 10 response characters
   if (! answer.empty()){
         phase = atoi(answer.substr(5,2).c_str());
   }
}

int AladdinController::HandleErrors()
{
   int lastError = error_;
   error_ = 0;
   return lastError;
}


/////////////////////////////////////
//  Communications
/////////////////////////////////////
void AladdinController::Send(string cmd)
{
   int ret = SendSerialCommand(port_.c_str(), cmd.c_str(), carriage_return);
   if (ret!=DEVICE_OK)
      error_ = DEVICE_SERIAL_COMMAND_FAILED;
}


void AladdinController::ReceiveOneLine()
{
   buf_string_ = "";
   GetSerialAnswer(port_.c_str(), line_feed, buf_string_);
   std::ofstream log;
   log.open ("Log_Pump.txt", std::ios::app);
   log << "Line received: " << buf_string_ << "\n";
   log.close();
}

void AladdinController::Purge()
{
   int ret = PurgeComPort(port_.c_str());
   if (ret!=0)
      error_ = DEVICE_SERIAL_COMMAND_FAILED;
}

void AladdinController::GetUnterminatedSerialAnswer (std::string& ans, unsigned int count)
{
	const unsigned long MAX_BUFLEN = 20; 
	char buf[MAX_BUFLEN];
	unsigned char* cc = new unsigned char[MAX_BUFLEN];
	int ret = 0;
    unsigned long read = 0;
    MM::MMTime startTime = GetCurrentMMTime();
    // Time out of 1 sec.  Make this a property?
    MM::MMTime timeOut(1000000);
	unsigned long offset = 0;
	while((offset < count) && ( (GetCurrentMMTime() - startTime) < timeOut))
	{
		ret = ReadFromComPort(port_.c_str(), cc, MAX_BUFLEN,read);
		for(unsigned int i=0; i<read; i++)      
		{
			buf[offset+i] = cc[i];
		}
		offset += read;
	}
	ans = buf;
}
