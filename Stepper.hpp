#pragma once

#include "Arduino.h"
#include "Configuration.hpp"

// Descreption
// This implements a basic interface for a stepper with a astepper driver, 
// This essentially only "steps" the motor alongside with setting its direction
// so this can be used with EasyDriver, DRV8825, L297 .. essentially anything
// with a STEP, DIR , EN pin .. doing the microstepping/revouloution calculations
// is the job of user

namespace Driver
{

class StepperMotor
{
  public:
    StepperMotor(char t_stepPin, char t_direcPin, char t_enPin);

    void Step();
    void SetDirection(char t_direction);

    void Enable(bool t_en);

  private :

    char m_stepPin = 0;
    char m_dirPin = 0;
    char m_enPin = 0;
};

}

