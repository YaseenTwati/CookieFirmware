#pragma once

#include "Arduino.h"
#include "Configuration.hpp"

// GCode Parser
// Functions for parsing GCode Lines and Parameter Seperation,
// * Supported Parameters are [ X, Y, Z, E, F, S, P, T ]
// * Lines Number [N###] are supported
// * Checksum validation is supported

// Supported GCODE Commands :
// G00 : Linear Movement        ( X# Y# Z# E# F# )
// G01 : Linear Movement        ( X# Y# Z# E# F# )
// G04 : Wait                   ( P# S# )
// G28 : Home Axis(s)           ( X Y Z )
// G92 : Set Current Position   ( X# Y# Z# E# )

// M104 : Heat Extruder                     ( T# S# )
// M105 : Report Extruder/Bed Temperature
// M106 : Wait for Temerature to Stabelize  ( N/A )
// M109 : Heat Extruder ( Wait )            ( T# S# )
// M112 : Emergency STOP
// M114 : Get Current Position ( TODO : Fix )


namespace GCode
{

struct Parameters
{
#ifdef DEBUG_GCODE
  char* buffer;
#endif

  bool isInvalidComment = false;

  long lineNumber = -1;
  
  bool checksum = true; // true by default incase no checksum was sent
  
  char letter;
  char code;

  // we do not support relative positioning,
  // if a value was passed, it will never negative
  
  float x = -1;
  float y = -1;
  float z = -1;
  float e = -1;

  float f = -1;
  float s = -1;
  float p = -1;
  float t = -1;
};


Parameters Parse(char* t_buffer);

void Dump(Parameters t_param);

}

