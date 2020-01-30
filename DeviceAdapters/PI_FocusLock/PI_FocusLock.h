///////////////////////////////////////////////////////////////////////////////
// FILE:          PI_FL.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   PI Focus Lock
//
// AUTHOR:        
// COPYRIGHT:     
// LICENSE:       
//

#ifndef _PI_FL_H_
#define _PI_FL_H_

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_PORT_CHANGE_FORBIDDEN    10004
#define ERR_UNRECOGNIZED_ANSWER      10009
#define ERR_OFFSET 10100

class PIMonitorThread;

class PIZStage : public CStageBase<PIZStage>
{
public:
   PIZStage();
   ~PIZStage();
  
   // Device API
   // ----------
   int Initialize();
   int Shutdown();
  
   void GetName(char* pszName) const;
   bool Busy();

   // Stage API
   // ---------
  int SetPositionUm(double pos);
  int GetPositionUm(double& pos);
  int SetPositionSteps(long steps);
  int GetPositionSteps(long& steps);
  int SetOrigin();
  int GetLimits(double& min, double& max);

 // int GetIntSensorPosition(double* pos);												
  int Set2External();																					////
  int Set2Internal();
  int SetServoState(int state);
  int ReportStateChange(double newState);

  // int StopThread();
  // int StartThread();

  int IsStageSequenceable(bool& isSequenceable) const {isSequenceable = false; return DEVICE_OK;}
  bool IsContinuousFocusDrive() const {return false;}

  
   // action interface
   // ----------------
  // int OnMonitoring(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSensorState(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnIntSensorPosition(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnStepSizeUm(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAxisName(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAxisLimit(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnPosition(MM::PropertyBase* pProp, MM::ActionType eAct);


private:
   int ExecuteCommand(const std::string& cmd, std::string& response);
   bool GetValue(std::string& sMessage, double& dval);
   bool GetValue(std::string& sMessage, long& lval);
   bool ExtractValue(std::string& sMessage);
   int GetError();
   bool waitForResponse();

   //PIMonitorThread* mThread_;
   std::string port_;
   std::string axisName_;
   bool checkIsMoving_;
   bool locked_;
   double stepSizeUm_;
   bool initialized_;
   long curSteps_;
   double answerTimeoutMs_;
   double axisLimitUm_;
};


/*
class PIMonitorThread : public MMDeviceThreadBase
{
   public:
      PIMonitorThread(PIZStage& PI, bool debug);
     ~PIMonitorThread();
      int svc();										
      int open (void*) { return 0;}
      int close(unsigned long) {return 0;}

      void Start();
      void Stop() {stop_ = true;}
      PIMonitorThread & operator=( const PIMonitorThread & ) 
      {
         return *this;
      }


   private:
      MM_THREAD_HANDLE thread_;
      long state_;
      PIZStage& PI_;
      bool debug_;
      bool stop_;
};
*/

#endif //_PI_FL_H_
