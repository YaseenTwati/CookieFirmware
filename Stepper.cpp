#include "Stepper.hpp"

// -----------------------------------------------

Driver::StepperMotor::StepperMotor(char t_stepPin, char t_dirPin, char t_enPin)

                                    : m_stepPin(t_stepPin), 
                                      m_dirPin(t_dirPin), 
                                      m_enPin(t_enPin)
{
  pinMode(m_stepPin, OUTPUT);
  pinMode(m_dirPin, OUTPUT);
  pinMode(m_enPin, OUTPUT);
}

// -----------------------------------------------

void Driver::StepperMotor::Step()
{
  digitalWrite(m_stepPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(m_stepPin, LOW);
  delayMicroseconds(5);
}

// -----------------------------------------------

void Driver::StepperMotor::SetDirection(char t_direction)
{
  digitalWrite(m_dirPin, t_direction);
}

// -----------------------------------------------

void Driver::StepperMotor::Enable(bool t_en)
{
  digitalWrite(m_dirPin, (char)t_en);
}

// -----------------------------------------------
