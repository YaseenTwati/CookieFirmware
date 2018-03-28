// -----------------------------------------------------------------------------------
//  Hardware and Firmware Configurations
// -----------------------------------------------------------------------------------

// ------------------------------------
//  GCode
// ------------------------------------
#define COMMENT_SAFE // Without this, wrong values may be read from GCODE if it had a comment with X,Y,Z,E,F,S,P ...
#define LOOKFOR_CHECKSUM
// Note : Inline Comments are not supported

#define SKIP_RS // skip corrupted line and do not ask for resend

// ------------------------------------
//  Movement ( Steps/mm )
// ------------------------------------
#define X_STEPS_MM 180.0
#define Y_STEPS_MM 180.0
#define Z_STEPS_MM 800.0
#define E_STEPS_MM 241.0

// ------------------------------------
//  Temperature
// ------------------------------------

#define NTC_BETA  3500  // NTC Thermistor Beta Value
#define NTC_PULLUP_R  4700
#define NTC_TYPE  100E3 // 100K

// PID "Related" Settings --------

#define PID_KP 14.95
#define PID_KI 0
//#define PID_KI 17.4
//#define PID_KI 0.7
//#define PID_KI 1.8

#define PID_KD 122

#define PID_OUT_MAX 254

#define PID_ACTIVE_RANGE 100 // This will vary if using a different supply voltage for the heater
#define HEATER_HEATUP_POWER 145 // 1 - 254

// ---------------------

#define HEATER_STABLE_SAMPLE_TIME 6000 // 6 seconds
#define HEATER_STABLE_SAMPLE_COUNT 6
#define HEATER_DEADBAND 3 // tolerence for temperature

// ------------------------------------
//  Pin Configuration
// ------------------------------------

// Control board Selection
//#define CUSTOM_BOARD
#define RAMPS_14

#ifdef RAMPS_14 // -----------------------------------------------

#define X_STEP_P  54
#define X_DIR_P   55
#define X_EN_P    38

#define Y_STEP_P  60
#define Y_DIR_P   61
#define Y_EN_P    56

#define Z_STEP_P  46
#define Z_DIR_P   48
#define Z_EN_P    62

#define E_STEP_P  26
#define E_DIR_P   28
#define E_EN_P    24

#define X_MIN     3
#define Y_MIN     14
#define Z_MIN     18

#define HOTEND_P  8
#define HOTEND_NTC_P     13

#define BED_P 0
#define BED_NTC_P 0

#elif defined(CUSTOM_BOARD) // -----------------------------------------------

#define X_STEP_P  2
#define X_DIR_P   A3
#define X_EN_P    4

#define Y_STEP_P  5
#define Y_DIR_P   6
#define Y_EN_P    7

#define Z_STEP_P  8
#define Z_DIR_P   9
#define Z_EN_P    10

#define E_STEP_P  11
#define E_DIR_P   12
#define E_EN_P    13

#define X_MIN     A0
#define Y_MIN     A1
#define Z_MIN     A2

#define HOTEND_P  3 // has to be PWM Pin
#define HOTEND_NTC_P     A4

// disabled for now ..
#define BED_P 0
#define BED_NTC_P 0

#endif // -----------------------------------------------

// ------------------------------------
//  Other Settings
// ------------------------------------
#define SERIAL_BAUDRATE 19200
#define MAX_SERIAL_LINE_LENGTH 156 // FATAL Error if this length was passed, Kill() will be called

#define PULSE_TIME 10 // stepper motor pulse time ( the time it takes to make a step ( HIGH .. LOW ) )

#define HOMEING_SPEED_X 20
#define HOMEING_SPEED_Y 20
#define HOMEING_SPEED_Z 20

#define INITIAL_FEEDRATE 40

#define DISALLOW_EXTRUDER_RETRACTION

//#define EXTRUDER_FLOATING_POINT_PRECISION

// ------------------------------------
//  Debugging Stuff
// ------------------------------------

#define START_MESSAGE // Banner

//#define DEBUG // This may create some time overhead in production .. keep off if not needed

//#define DEBUG_PID

//#define DEBUG_H_STABLEIZATION

//#define DISABLE_MOVEMENT // This does not effect homing

//#define DEBUG_GCODE
//#define DBUG_GCODE_2

//#define DEBUG_STEPS

//#define DEBUG_POSITION
//#define DEBUG_POSITION_EVERYLINE

//#define DEBUG_FEEDRATE

//#define DEBUG_HEAT
//#define DEBUG_HEAT_2

//#define DEBUG_CHECKSUM
