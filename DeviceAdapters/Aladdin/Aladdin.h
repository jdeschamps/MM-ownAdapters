///////////////////////////////////////////////////////////////////////////////
// FILE:          Aladdin.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Aladdin pump controller adapter
// COPYRIGHT:     University of California, San Francisco, 2011
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
// AUTHOR:        Kurt Thorn, UCSF, November 2011
//				  Joran Deschamps and Thomas Chartier, EMBL, May 2016
//

#ifndef _ALADDIN_H_
#define _ALADDIN_H_

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/DeviceUtils.h"
#include <string>
//#include <iostream>
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
//#define ERR_UNKNOWN_POSITION         10002
#define ERR_PORT_CHANGE_FORBIDDEN    10004

//enum TriggerType {OFF, RISING_EDGES, FALLING_EDGES, BOTH_EDGES, FOLLOW_PULSE};
//string TriggerLabels[] = {"Off","RisingEdges","FallingEdges","BothEdges","FollowPulse"};
//char TriggerCmd[] = {'Z', '+', '-', '*', 'X'};

class AladdinController : public CGenericBase<AladdinController>
{
public:
   AladdinController(const char* name);
   ~AladdinController();
  
   // MMDevice API
   // ------------
   int Initialize();
   int Shutdown();
  
   void GetName(char* pszName) const;
   bool Busy();
   
   // action interface
   // ----------------
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnVolume(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnDiameter(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnRate(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnDirection(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnRun(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnPhase(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnFunction(MM::PropertyBase* pProp, MM::ActionType eAct, long pump);
   int OnPumpNumber(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool initialized_;
   std::string name_;
   int error_;
   MM::MMTime changedTime_;
   long Npumps_;
   std::string port_;
   string buf_string_;
   vector<string> buf_tokens_;

   void SetVolume(long pump, double volume);
   void GetVolume(long pump, double& volume);
   
   void SetDiameter(long pump, double diameter);
   void GetDiameter(long pump, double& diameter);
   
   void SetRate(long pump, double rate);
   void GetRate(long pump, double& rate);
   
   void SetRun(long pump, long run);
   void GetRun(long pump, long& run);
   
   void GetDirection(long pump, string& direction);
   void SetDirection(long pump, string direction);

   void GetFunction(long pump, string& function);
   void SetFunction(long pump, string function);
   
   void SetPhase(long pump, long phase);
   void GetPhase(long pump, long& phase);

   bool isValidFunction(string function);

   void StripString(string& StringToModify);
   void SendPurge(string s);
   void Send(string cmd);
   void ReceiveOneLine();
   void Purge();
   void GetUnterminatedSerialAnswer (std::string& ans, unsigned int count);
   int HandleErrors();

   static const int RCV_BUF_LENGTH = 1024;

   AladdinController& operator=(AladdinController& /*rhs*/) {assert(false); return *this;}
};


#endif // _ALADDIN_H_
