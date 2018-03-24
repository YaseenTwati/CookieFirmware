#include "Cookie.hpp"

// -----------------------------------------------

void Cookie::InitializeCookieFirmware()
{
  // Initialize Serial
  Serial.begin(SERIAL_BAUDRATE);

  // Pullup Endstops
  digitalWrite(X_MIN, HIGH);
  digitalWrite(Y_MIN, HIGH);
  digitalWrite(Z_MIN, HIGH);

  // Set heaters as outputs
  pinMode(HOTEND_P, OUTPUT);
  pinMode(BED_P, OUTPUT);

  // Initialize Timer1 Settings for Temperture and PID
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 8572; // 2Hz ( every 500ms )

  TCCR1B |= (1 << CS12 ); // 256 prescaler

  TIMSK1 |= (1 << TOIE1); // Timer 1 Overflow Interrupt


#ifdef START_MESSAGE
  // Print Welcome Message
  Serial.println("//COOKIE FIRMWARE V1");
#endif
}

// -----------------------------------------------
// Stepper Motors

Driver::StepperMotor stepperX(X_STEP_P, X_DIR_P, X_EN_P);
Driver::StepperMotor stepperY(Y_STEP_P, Y_DIR_P, Y_EN_P);
Driver::StepperMotor stepperZ(Z_STEP_P, Z_DIR_P, Z_EN_P);
Driver::StepperMotor stepperE(E_STEP_P, E_DIR_P, E_EN_P);

// -----------------------------------------------
//  Currunt Position/Feedrate

float currentX = 0.0;
float currentY = 0.0;
float currentZ = 0.0;
float currentE = 0.0;

float currentFeedrate = INITIAL_FEEDRATE;

// -----------------------------------------------
//  Heaters and current Ttemperature targets

Temperature::Heater heater_hotend(HOTEND_NTC_P, HOTEND_P);

float currentHotEndTarget = 0.0;

#ifdef ENABLE_HEATEDBED
Temperature::Heater heater_hotend(BED_NTC_P, BED_P);

float currentedTarget = 0.0;
#endif

// -----------------------------------------------
//  Main Control Loop

void Cookie::CookieMainLoop()
{
  unsigned int index = 0;
  char* lineBuffer = new char[MAX_SERIAL_LINE_LENGTH];

  while (1)
  {
    while (Serial.available() > 0)
    {
      if (index >= MAX_SERIAL_LINE_LENGTH)
      {
        // Fata Error
        Kill();
      }

      // Line End ?
      char character = Serial.read();
      if (character == '\r' || character == '\n')
      {
        // Terminate the string
        lineBuffer[index] = '\0';
        if (index > 0)
        {
          index = 0;
          GCode::Parameters params = GCode::Parse(lineBuffer);

          // Line was not recieved correctly .. as for resend
          if (!params.checksum)
          {
#ifdef SKIP_RS// skip the line ?
            Serial.println("ok");

            // update the e position so we dont "over extrude" on the next step
            if (params.e != -1)
            {
              currentE = params.e;
            }

            continue;
#else
            Serial.print("rs "); Serial.println(params.lineNumber);
            continue;
#endif
          }

#ifdef DBUG_GCODE_2
          Serial.println(lineBuffer);
#endif

#ifdef DEBUG_GCODE
          GCode::Dump(params);
#endif
          ExecuteLine(params);
        }
      }
      else
      {
        // Append character to buffer
        lineBuffer[index] = character;
        index++;
      }
    }
  }
}

// -----------------------------------------------

void Cookie::ExecuteLine(GCode::Parameters t_params)
{

#ifdef DEBUG_POSITION_EVERYLINE
  Serial.print("X : "); Serial.print(currentX); Serial.print("   Y : "); Serial.print(currentZ); Serial.print("   Z : "); Serial.print(currentZ); Serial.print("   E : "); Serial.println(currentE);
#endif

  bool okSent = false; // Some command functions send their own "ok"s. ex : M105

  if (t_params.letter == 'G')
  {
    switch (t_params.code)
    {
      case 28 : // Home Axis
        {
          // check if a parameter is provided, we do not care about if it had a vlue or not
          bool homeX = (t_params.x > 0);
          bool homeY = (t_params.y > 0);
          bool homeZ = (t_params.z > 0);

          // if no parameters are provided that means home ALL
          if (!homeX && !homeY && !homeZ)
          {
            homeX = true;
            homeY = true;
            homeZ = true;
          }

          HomeAxis(homeX, homeY, homeZ);

          break;
        }

      case 0 :
      case 1 : // Linear Movement ( Only Absolute positioning is supported )
        {
          if (t_params.f != -1)
          {
            currentFeedrate = t_params.f / 60; // Feedrate is provided in mm/min
#ifdef DEBUG_FEEDRATE
            Serial.print("Feedrate Changed to : "); Serial.println(currentFeedrate);
#endif
          }

          // Draw the line ..
          PerformG1(t_params.x, t_params.y, t_params.z, t_params.e);

          // Update current position

          if (t_params.x != -1)
            currentX = t_params.x;
          if (t_params.y != -1)
            currentY = t_params.y;
          if (t_params.z != -1)
            currentZ = t_params.z;
          if (t_params.e != -1)
            currentE = t_params.e;

#ifdef DEBUG_POSITION
          Serial.print("X : "); Serial.print(currentX); Serial.print("   Y : "); Serial.print(currentZ); Serial.print("   Z : "); Serial.print(currentZ); Serial.print("   E : "); Serial.println(currentE);
#endif

          break;
        }

      case 4 :  // Wait
        {
          // Milliseconds
          if (t_params.p != -1)
          {
            delay(t_params.p);
          }
          // Seconds
          else if (t_params.s != -1)
          {
            delay(1000 * t_params.s);
          }

          break;
        }

      case 92 : // Update Current Position
        {
          if (t_params.x != -1) currentX = t_params.x;
          if (t_params.y != -1) currentY = t_params.y;
          if (t_params.z != -1) currentZ = t_params.z;
          if (t_params.e != -1) currentE = t_params.e;

          break;
        }

      default : // Some Command We dont dsupport ( Yet ? )
        {
#ifdef DEBUG
          Serial.print("Unsupported G Command : [G"); Serial.print(t_params.code); Serial.println("]");
#endif
        }

    }
  }
  else // M
  {
    switch (t_params.code)
    {
      case 104 :  // Heat Extruder ( Hotend )
        {
#ifdef DEBUG_HEAT
          Serial.print("Extruder Temp Target Changed : "); Serial.println(t_params.s);
#endif
          heater_hotend.SetTemperature(t_params.s);
          currentHotEndTarget = t_params.s;

          break;
        }

      case 106 :  // Stabelize Temperature
        {
          if (currentHotEndTarget != 0)
          {
            heater_hotend.Stabelize();
          }
          break;
        }

      case 109 :  // Heat Extruder and WAIT for it t heat up
        {
          heater_hotend.SetTemperature(t_params.s);
          heater_hotend.Stabelize();
        }

        // TODO : Fix This
#ifdef ENABLE_HEATEDBED

      case 111 :
        {

          break;
        }
#endif


      case 105 : // Get Current Temperature
        {
          ReportTemperature();
          okSent = true;
          break;
        }

      case 112 : // EMERGENCY STOP
        {
          Kill();
          break;
        }

      case 114 : // Get Current Position
        {
          ReportPosition();
          okSent = true;
          break;
        }

      case 119 : // Report Endstop Status
        {
          bool xEnd = digitalRead(X_MIN);
          bool yEnd = digitalRead(Y_MIN);
          bool zEnd = digitalRead(Z_MIN);

          Serial.print("//X Min Enstop : "); Serial.println(xEnd);
          Serial.print("//Y Min Enstop : "); Serial.println(yEnd);
          Serial.print("//Z Min Enstop : "); Serial.println(zEnd);

          break;
        }

      default :
        {
#ifdef DEBUG
          Serial.print("Unsupported M Command : [M"); Serial.print(t_params.code); Serial.println("]");
#endif
        }

    }
  }

  if (!okSent)
    Serial.println("ok");
}

// -----------------------------------------------

void Cookie::PerformG1(float t_x, float t_y, float t_z, float t_e)
{
  unsigned long start_t  = 0;
  unsigned long end_t = 0;

  // TL;DR
  // Calculate deltas of every axis
  // Set Motor Directions based on current position or target position is bigger
  // Calculate Distance based on the X Y plane
  // Calculate The Time based on that distance and the feed rate we have
  // Calculat the number of steps for every axis
  // Calculate the delays ( speed ) based on the number of steps of every axis
  // Loop and steps motors untill all done

  // if something is -1 that means it has been left as is and should not be moved
  if (t_x == -1) t_x = currentX;
  if (t_y == -1) t_y = currentY;
  if (t_z == -1) t_z = currentZ;
  if (t_e == -1) t_e = currentE;


  // do not allow the extruder to go backwards
#ifdef DISALLOW_EXTRUDER_RETRACTION
  if (t_e < currentE)
    t_e = currentE;
#endif


  // Calculate Deltas ---------------

  float dx = abs(t_x - currentX);
  float dy = abs(t_y - currentY);
  float dz = abs(t_z - currentZ);
  float de = abs(t_e - currentE);

  // Set Motor Directions ------------

  if (t_x > currentX) stepperX.SetDirection(1); else stepperX.SetDirection(0);
  if (t_y > currentY) stepperY.SetDirection(1); else stepperY.SetDirection(0);
  if (t_z > currentZ) stepperZ.SetDirection(1); else stepperZ.SetDirection(0);
  if (t_e > currentE) stepperE.SetDirection(1); else stepperE.SetDirection(0);

  float duration;
  float distance;

  // are moving on the XY plane ? if so do the calculations of time based on the distance we have to move
  if (!(dx == 0 && dy == 0))
  {
    // Calculate The Distance and the time to cross it based on the speed

    distance = sqrtf( dx * dx   + dy * dy  ); // Distance between two points equation

    duration = distance / currentFeedrate;
    duration *= 1E6; // s -> uS
  }
  else
  {
#ifdef DEBUG
    Serial.println("Not Moving on X/Y");
#endif
    // We are only moving the Z or E
    if (dz)
    {
      duration = dz / currentFeedrate;
    }
    else
    {
      duration = de / currentFeedrate;
    }

    duration *= 1E6;
  }
  // Calculate The Total Number of Steps and the required delay between steps ---------------

  unsigned long totalStepsX = (dx * X_STEPS_MM );
  unsigned long totalStepsY = (dy * Y_STEPS_MM );
  unsigned long totalStepsZ = (dz * Z_STEPS_MM );
  unsigned long totalStepsE = (de * E_STEPS_MM );

  unsigned long delayTimeX =  ( duration / totalStepsX );
  unsigned long delayTimeY =  ( duration / totalStepsY );
  unsigned long delayTimeZ =  ( duration / totalStepsZ );
  unsigned long delayTimeE =  ( duration / totalStepsE );

  unsigned long steppedX = 0;
  unsigned long steppedY = 0;
  unsigned long steppedZ = 0;
  unsigned long steppedE = 0;

  unsigned long steppedAll = 0;
  unsigned long totalStepsAll = totalStepsX + totalStepsY + totalStepsZ + totalStepsE;

  // Debugging Stuff ------------------------

#ifdef DEBUG_STEPS

  Serial.println("------------------------------------");

  Serial.print("CurrentX : "); Serial.print(currentX); Serial.print(" |  Target X : "); Serial.println(t_x);
  Serial.print("CurrentY : "); Serial.print(currentY); Serial.print(" |  Target Y : "); Serial.println(t_y);
  Serial.print("CurrentZ : "); Serial.print(currentZ); Serial.print(" |  Target Z : "); Serial.println(t_z);
  Serial.print("CurrentE : "); Serial.print(currentE); Serial.print(" |  Target E : "); Serial.println(t_e);

  Serial.println("----------------------");

  Serial.print("Dx : "); Serial.println(dx);
  Serial.print("Dy : "); Serial.println(dy);
  Serial.print("Dz : "); Serial.println(dz);
  Serial.print("De : "); Serial.println(de);

  Serial.println("----------------------");

  Serial.print("Distance : "); Serial.println(distance);
  Serial.print("Duration : "); Serial.println(duration);
  Serial.print("Feedrate : "); Serial.println(currentFeedrate);

  Serial.println("------------------------------------\n");

  //Serial.print("Total Steps X : "); Serial.println(totalStepsX);
  //Serial.print("Total Steps Y : "); Serial.println(totalStepsY);
  //Serial.print("Total Steps Z : "); Serial.println(totalStepsZ);
  //Serial.print("Total Steps E : "); Serial.println(totalStepsE);

  //Serial.print("DelayTime X : "); Serial.println(delayTimeX);
  //Serial.print("DelayTime Y : "); Serial.println(delayTimeY);
  //Serial.print("DelayTime Z : "); Serial.println(delayTimeZ);
  //Serial.print("DelayTime E : "); Serial.println(delayTimeE);

#endif

#ifndef DISABLE_MOVEMENT

  // Step Motors ----------------------------------

  unsigned long now = micros();

  unsigned long lastX = now;
  unsigned long lastY = now;
  unsigned long lastZ = now;
  unsigned long lastE = now;

  while (steppedAll < totalStepsAll)
  {
    now  = micros();

    if (steppedX < totalStepsX)
    {
      if ((now - lastX) >= delayTimeX)
      {
        stepperX.Step();
        lastX = now;
        steppedAll++;
        steppedX++;
      }
    }

    if (steppedY < totalStepsY)
    {
      if ((now - lastY) >= delayTimeY)
      {
        stepperY.Step();
        lastY = now;
        steppedY++;
        steppedAll++;
      }
    }

    if (steppedZ < totalStepsZ)
    {
      if ((now - lastZ) >= delayTimeZ)
      {
        stepperZ.Step();
        lastZ = now;
        steppedZ++;
        steppedAll++;
      }
    }

    if (steppedE < totalStepsE)
    {
      if ((now - lastE) >= delayTimeE)
      {
        stepperE.Step();
        lastE = now;
        steppedE++;
        steppedAll++;
      }
    }
  }

#endif
}

// -----------------------------------------------

void Cookie::HomeAxis(bool t_hx, bool t_hy, bool t_hz)
{
  // TLD:DR
  // Calculate neccesarry delays based on axis homing speeds
  // Step Motors

  unsigned long delayTimeX =  ( 1000000 -  ( PULSE_TIME * (HOMEING_SPEED_X * X_STEPS_MM) ) ) / (HOMEING_SPEED_X * X_STEPS_MM);
  unsigned long delayTimeY =  ( 1000000 -  ( PULSE_TIME * (HOMEING_SPEED_Y * Y_STEPS_MM) ) ) / (HOMEING_SPEED_Y * Y_STEPS_MM);
  unsigned long delayTimeZ =  ( 1000000 -  ( PULSE_TIME * (HOMEING_SPEED_Z * Z_STEPS_MM) ) ) / (HOMEING_SPEED_Z * Z_STEPS_MM);

#ifdef DEBUG
  Serial.println((int)t_hx);
  Serial.println(delayTimeX);
  Serial.println((int)t_hx);
  Serial.println(delayTimeY);
  Serial.println((int)t_hx);
  Serial.println(delayTimeZ);
#endif

  // Make sure we are homing in the right direction ...
  stepperX.SetDirection(0);
  stepperY.SetDirection(0);
  stepperZ.SetDirection(0);

  if (t_hx)
  {
    while ( digitalRead(X_MIN) )
    {
      stepperX.Step();
      delayMicroseconds(delayTimeX);
    }

    currentX = 0;
  }

  if (t_hy)
  {
    while ( digitalRead(Y_MIN) )
    {
      stepperY.Step();
      delayMicroseconds(delayTimeY);
    }

    currentY = 0;
  }

  if (t_hz)
  {
    while ( digitalRead(Z_MIN) )
    {
      stepperZ.Step();
      delayMicroseconds(delayTimeZ);
    }

    currentZ = 0;
  }
}

// -----------------------------------------------

void Cookie::Kill()
{
  // Turn Off Steppers
  stepperX.Enable(false);
  stepperY.Enable(false);
  stepperZ.Enable(false);
  stepperE.Enable(false);

  // Disable Heaters
  heater_hotend.SetTemperature(0.0);

#ifdef ENABLE_HEATEDBED

  heater_bed.SetTemperature(0.0);

#endif

  Serial.println("// FATAL ERROR ... KILLING");
  Serial.println("!!");

  // Loop Forever
  for (;;) {}
}

// -----------------------------------------------

void Cookie::ReportTemperature(bool t_comment)
{
  float hotend_temp = heater_hotend.GetCurrentTemp();
  float bed_temp = -273.0; // this should be returned if we dont have a bed

#ifdef ENABLE_HEATEDBED
  bed_temp = heater_bed.GetCurrentTemp();
#endif

  if (!t_comment)
  {
    Serial.print("ok T:"); Serial.print(hotend_temp); Serial.print(" B: "); Serial.println(bed_temp);
  }
  else
  {
    Serial.print("// T:"); Serial.print(hotend_temp); Serial.print(" B: "); Serial.println(bed_temp);
  }

}

// -----------------------------------------------

void Cookie::ReportPosition()
{
  // TODO
}

// -----------------------------------------------

ISR(TIMER1_OVF_vect)
{
  heater_hotend.Heat();

#ifdef ENABLE_HEATEDBED
  heater_bed.Heat();
#endif
}

// -----------------------------------------------

void Cookie::DevelopmentInit()
{
  heater_hotend.SetTemperature(220);
}

// -----------------------------------------------

void Cookie::DevelopmentLoop()
{
  Serial.print("Temerature : "); Serial.println(heater_hotend.GetCurrentTemp());
}

// -----------------------------------------------

