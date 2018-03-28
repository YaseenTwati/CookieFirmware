// -----------------------------------------------------------------------------------
//
//  Cookie Firmware : 3D Printer Firmware Implementation
//  Copyright(C) Yaseen Mohamed Twati - 2018
//  This Firmware has been written from scratch without relaying on any Libraries,
//  
//  Why the name Cookie? Well, Why not lol
//
//  This Firmware implements the basic needed features to get a 3D Printer working
//
//  Supported GCodes : G00 , G01, G4, G28, G92, M104, M109, M106, M119, M112, M114, M105
//
// -----------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------
//
//  Files :
//    Configuration.hpp : General Configuration
//    GCode.cpp/hpp     : GCode Parsing and parameter seperation
//    Stepper.cpp/hpp   : DRV8825 Stepper Motor Interface
//    Cookie.cpp/hpp    : Main control loop and movement implementation
//    Temp.cpp/hpp      : Temperature Control, NTC, and PID things
//
//  C++ Namespaces :
//    Cookie::      : Main Control Functions
//    Temperature:: : Temperature related classes ( Thermistor, PID, Heater ... )
//    GCode::       : GCode Parsing Stuff
//
// -----------------------------------------------------------------------------------

#include "Cookie.hpp"

// ----------------------------------------------

void setup()
{
  Cookie::InitializeCookieFirmware();
  //Cookie::DevelopmentInit();
}

// ----------------------------------------------

void loop()
{
  Cookie::CookieMainLoop();
  //Cookie::DevelopmentLoop();
}

// ----------------------------------------------
