#pragma once

#include "Arduino.h"

#include "Temp.hpp"
#include "GCode.hpp"
#include "Stepper.hpp"
#include "Configuration.hpp"

#include <math.h>

#define POW_2(x) (x * x)

namespace Cookie
{

/*
  Initialize Internel stuff, pinModes, Serial, Pullups .... etc 
*/
void InitializeCookieFirmware();

/*
  Execute a line of GCode.
*/
void ExecuteLine(GCode::Parameters);

/*
  Perform G1 Operation ( Linear Movement )
*/
void PerformG1(float t_x, float t_y, float t_z, float t_e);

/*
  Home one or more axis 
*/
void HomeAxis(bool t_hx, bool t_hy, bool t_hz);

/*
  Report the temperature to the control host ( PC )
*/
void ReportTemperature(bool t_comment = false);

/*
  Report Position to the control host ( PC )
*/
void ReportPosition();

/*
  Main Control Loop
*/
void CookieMainLoop();

/*
  Disable Everything, Stop Printer, Loop Forver
*/
void Kill();


/*
  Just a Function that is used to test stuff in development ( to be easily called from loop() without messing with CookieMainLoop()
*/
void DevelopmentInit();

void DevelopmentLoop();

}
