#include "Temp.hpp"

// ----------------------------------------------------------------------------------------------
//  PID
// ----------------------------------------------------------------------------------------------

Temperature::PID::PID()
{
}

Temperature::PID::PID(float t_Kp, float t_Ki, float t_Kd, float t_outMin, float t_outMax)
  : m_Ki(t_Ki),
    m_Kp(t_Kp),
    m_Kd(t_Kd),
    m_outMax(t_outMax),
    m_outMin(t_outMin)
{
}

// -----------------------------------------------

void Temperature::PID::SetParameters(float t_Kp, float t_Ki, float t_Kd, float t_outMin, float t_outMax)
{
  m_Ki = t_Ki;
  m_Kp = t_Kp;
  m_Kd = t_Kd;
  m_outMax = t_outMax;
  m_outMin = t_outMin;
}

// -----------------------------------------------

void Temperature::PID::SetTarget(float t_setPoint)
{
  m_setPoint = t_setPoint;
}

// -----------------------------------------------

// Sorry Styling Guidelines xD .. i just had to use capital letters for local P, I and D variables
// it just looks much better here xD ... lol
float Temperature::PID::Compute(float t_input)
{
  float now = millis();
  now /= 1000;

  float timeDifference = now - m_lastRun;

  float error = m_setPoint - t_input;

  m_integralSum += ( error * timeDifference );

  float derivative = ( error - m_lastError ) / timeDifference;

  float output = m_Kp * error + m_Ki * m_integralSum + m_Kd * derivative;

  if (output > m_outMax) output = m_outMax;
  else if (output < m_outMin) output = m_outMin;

  m_lastError = error;
  m_lastRun = now;

#ifdef DEBUG_PID
  Serial.print("Erorr = "); Serial.println(error);
  Serial.print("P :  "); Serial.println(m_Kp * error);
  Serial.print("I :  "); Serial.println(m_Ki * m_integralSum);
  Serial.print("D :  "); Serial.println(m_Kd * derivative);
  Serial.print("Output :  "); Serial.println(output);
#endif

  return output;
}

// ----------------------------------------------------------------------------------------------
//  Thermistor
// ----------------------------------------------------------------------------------------------

Temperature::Thermistor::Thermistor()
{
}

Temperature::Thermistor::Thermistor(char t_ntc_pin, int t_ntc_type, int t_ntc_beta, int t_other_r)
  : m_ntc_pin(t_ntc_pin),
    m_ntc_type(t_ntc_type),
    m_ntc_beta(t_ntc_beta),
    m_other_r(t_other_r)
{
}

// -----------------------------------------------

void Temperature::Thermistor::SetParameters(char t_ntc_pin, int t_ntc_type, int t_ntc_beta, int t_other_r)
{
  m_ntc_pin = t_ntc_pin;
  m_ntc_type = t_ntc_type;
  m_ntc_beta = t_ntc_beta;
  m_other_r = t_other_r;
}

// -----------------------------------------------

float Temperature::Thermistor::Read()
{
  float adc = 0;

  for (char i = 0; i < 10; i++)
  {
    adc += analogRead(m_ntc_pin);
  }

  adc /= 10;

  float r  = (1023 / adc) - 1;
  r = m_other_r / r;

  // Apply Steinhart Equation to do the conversion to temperature

  float temp = r / NTC_TYPE;
  temp = log(temp);
  temp /= m_ntc_beta;
  temp += 1.0 / (25 + 273.15); // 25 is the temperature at the NTC_TYPE resistance
  temp = 1.0 / temp;
  temp -= 273.15;

#ifdef DEBUG
  Serial.print("ADC Average Readings : "); Serial.println(adc);
  Serial.print("Current NTC Resistance : "); Serial.println(r);
  Serial.print("Current NTC Temperature : "); Serial.println(temp);
#endif

  return temp;
}

// ----------------------------------------------------------------------------------------------
//  Heater
// ----------------------------------------------------------------------------------------------

Temperature::Heater::Heater(char t_ntc_pin, char t_heater_pin)
  : m_heater_pin(t_heater_pin)
{
  m_pidController.SetParameters(PID_KP, PID_KI, PID_KD, 0, PID_OUT_MAX);
  m_ntc.SetParameters(t_ntc_pin, NTC_TYPE, NTC_BETA, NTC_PULLUP_R);

  pinMode(t_heater_pin, OUTPUT);
}

void Temperature::Heater::SetTemperature(float t_temp)
{
  m_targetTemp = t_temp;
  m_pidController.SetTarget(t_temp);

  if (t_temp > GetCurrentTemp())
    m_rising = true;
}

void Temperature::Heater::Heat()
{
  if (m_targetTemp == 0)
  {
    analogWrite(m_heater_pin, 0);
    return;
  }

  float temp = GetCurrentTemp();

  //  we only want to apply the the  PID Active Zone thing if we are rising to higher temperature
  // and ONLY at the first, if we dont check for rising, the pid will stop working once we fall
  // below the target setpoint again

  if (m_rising)
  {
    // If we are hotter than our target temperature or we are within the
    // PID_ACTIVE_RANGE range, give the control to the PID controller
    if (temp > m_targetTemp || ( ( m_targetTemp - temp ) < PID_ACTIVE_RANGE ) )
    {
      int pwmOut = m_pidController.Compute(temp);
      analogWrite(m_heater_pin, pwmOut);
    }
    else
    {
      // Set Power to
      analogWrite(m_heater_pin, HEATER_HEATUP_POWER);
    }

    if (temp > m_targetTemp)
      m_rising = false;
  }
  else
  {
    int pwmOut = m_pidController.Compute(temp);
    analogWrite(m_heater_pin, pwmOut);
  }

#ifdef DEBUG_HEAT
  Serial.print("Current Temp : "); Serial.println(temp);
#endif
}

bool Temperature::Heater::IsStable()
{
  unsigned long start = millis();

  float samples[HEATER_STABLE_SAMPLE_COUNT];

  for (unsigned int i = 0; i < HEATER_STABLE_SAMPLE_COUNT; i++)
  {
    samples[i] = m_ntc.Read();
    delay(HEATER_STABLE_SAMPLE_TIME / HEATER_STABLE_SAMPLE_COUNT);
  }

#ifdef DEBUG_H_STABLEIZATION
  for (unsigned int i = 0; i < HEATER_STABLE_SAMPLE_COUNT; i++)
  {
    Serial.print("Heater Sample : "); Serial.println(samples[i]);
  }
#endif

  for (unsigned int i = 0; i < HEATER_STABLE_SAMPLE_COUNT; i++)
  {
    float temp_d = ( m_targetTemp - samples[i] );
    if (abs(temp_d > HEATER_DEADBAND ))
    {
#ifdef DEBUG_H_STABLEIZATION
      Serial.println("Heater Not Stable Yet ..");
#endif
      return false;
    }
  }

  return true;
}

void Temperature::Heater::Stabelize()
{
#ifdef DEBUG_H_STABLEIZATION
  Serial.println("[[[[[[[[[[[[[[[[[[ Temperature Stabilization Started ]]]]]]]]]]]]]]]]]]]");
#endif

  long now = millis();
  long lastT = now;
  // Essentially just wait until PID does its work ..
  while (!IsStable())
  {
    now = millis();
    if ((now - lastT) > 1000)
    {
      Cookie::ReportTemperature();
      lastT = now;
    }
  }

#ifdef DEBUG_H_STABLEIZATION
  Serial.println("[[[[[[[[[[[[[[[[[[[ Temperature Stabelized ]]]]]]]]]]]]]]]]]]");
#endif
}

float Temperature::Heater::GetCurrentTemp()
{
  return m_ntc.Read();
}

