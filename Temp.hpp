#pragma once

// I could've used the "Arduino PID" library, but this was more fo a learning experience 
// plus, i made the decision to not use any Library so I decided to write my own 
// .. even if it was the same thing .. Internet can be a great reference

// Description :
// PID Controller Implementation

#include "Arduino.h"
#include "Temp.hpp"
#include "Configuration.hpp"
#include "Cookie.hpp"

namespace Temperature
{

// --------------------------------------


class PID
{
  public :
  
    PID();
    PID(float t_Kp, float t_Ki, float t_Kd, float t_outMin, float t_outMax);

    void SetParameters(float t_Kp, float t_Ki, float t_Kd, float t_outMin, float t_outMax);

    void SetTarget(float t_setPoint);
    
    float Compute(float t_input);

  private :

    unsigned long m_lastRun = 0;

    float m_integralSum = 0;  // Used for I
    float m_lastInput = 0;
    
    float m_setPoint = 0;
    float m_lastError = 0;

    float m_Kp;
    float m_Ki;
    float m_Kd;

    float m_outMin;
    float m_outMax;
};

// --------------------------------------

class Thermistor
{
  public :
  
    Thermistor();
    Thermistor(char t_ntc_pin, int t_ntc_type, int t_ntc_beta, int t_other_r);

    void SetParameters(char t_ntc_pin, int t_ntc_type, int t_ntc_beta, int t_other_r);
    
    float Read();

  private :

    int m_ntc_pin = 0;
    int m_ntc_type = 0;
    int m_ntc_beta = 0;
    int m_other_r = 0;
};

// --------------------------------------

class Heater
{
  public :
    Heater(char t_ntc_pin, char t_heater_pin);

    void SetTemperature(float t_temp);

    void Heat();

    bool IsStable();

    void Stabelize();

    float GetCurrentTemp();

  private :

  char m_heater_pin = 0;
  
  PID m_pidController;
  
  Thermistor m_ntc;

  float m_targetTemp = 0.0;

  bool m_rising = true;
  
};

// --------------------------------------

}
