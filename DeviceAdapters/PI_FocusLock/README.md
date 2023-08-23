We use this device adapter to switch from internal to external sensors using a focus-locking system. The focus-locking system uses a 785 nm laser beam reflected in TIRF on a coverslip and 
detected by a quadrant photo-diode to compensate for axial drift. The QPD electronics sends a 0-10 V signal to the analog input of a E-709 stage controller (PI). The signal corresponds to -1/+1 um (internal parameters).
Our z stage is a P-726 (PI).

:warning: The device adapter sets some internal controller parameter values. These values might be only safe for our z stage. Please contact PI or your device manufacturer to the exact procedure to switch from internal to external sensor (and inversely). ​Us​e​ th​i​s ​d​e​vi​ce ​a​dap​te​r ​a​t ​y​o​ur​ ​ow​n ​ri​sk​.​ :warning:

Here are two snippet of code showing how the device adapter switches between internal and external sensors using **hard coded** values.

## Switch to external sensors

``` cpp 
string answer;

double low,high;

SetServoState(0);

// Set to external sensor
SendSerialCommand(port_.c_str(), "SPA Z 0x07000500 0", "\n");
SendSerialCommand(port_.c_str(), "SPA Z 0x07000501 1", "\n");

// Set servo parameters P,I
SendSerialCommand(port_.c_str(), "SPA Z 0x07000300 0.02", "\n");
SendSerialCommand(port_.c_str(), "SPA Z 0x07000301 2e-3", "\n");

// Set soft limits
SendSerialCommand(port_.c_str(), "VOL? 1", "\n");
GetSerialAnswer(port_.c_str(), "\n", answer);
		
answer=answer.substr(2,answer.length()-3);	
low=atof(answer.c_str())-25;					
high=low+50;

ostringstream command1,command2;
command1 << "SPA 1 0x0c000000 " << low;
command2 << "SPA 1 0x0c000001 " << high;
SendSerialCommand(port_.c_str(), command1.str().c_str(), "\n");
SendSerialCommand(port_.c_str(), command2.str().c_str(), "\n");
	
SetServoState(1);
```

## Switch to internal sensors

``` cpp
SetServoState(0);
				
// Set to internal sensor			
SendSerialCommand(port_.c_str(), "SPA Z 0x07000500 1", "\n");
SendSerialCommand(port_.c_str(), "SPA Z 0x07000501 0", "\n");

// Set servo parameters
SendSerialCommand(port_.c_str(), "SPA Z 0x07000300 0.02", "\n");
SendSerialCommand(port_.c_str(), "SPA Z 0x07000301 1.567286e-4", "\n");

// Set soft limits
SendSerialCommand(port_.c_str(), "SPA 1 0x0c000000 -30", "\n");
SendSerialCommand(port_.c_str(), "SPA 1 0x0c000001 130", "\n");
	
SetServoState(1);
```
