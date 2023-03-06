#pragma once

#include "StreamInterface.hh"
#include "BufferWriter.hh"
#include "CRC.hh"

namespace telem
{

// Format 0.3 documentation:
// https://docs.google.com/document/d/1TqxZ8coGGytWHkU9Wd6IoYnJqQGNaf_L90kL-XEnLwc/edit#

const uint8_t VERSION_MARKER_0_2 = 0x44; // Magic byte indicating version 0.2
const uint8_t VERSION_MARKER_0_3 = 0x45; // Magic byte indicating version 0.3

const uint8_t MSG_TAG_FRAME                   = 0x01; // 1
const uint8_t MSG_TAG_SOURCE_ID               = 0x02; // 2
const uint8_t MSG_TAG_SEQUENCE                = 0x03; // 3
const uint8_t MSG_TAG_JUMP_TO_BOOT            = 0x05; // 5
const uint8_t MSG_TAG_SOFTWARE_VERSION        = 0x10; // 16
const uint8_t MSG_TAG_TIME_EPOCH              = 0x11; // 17
const uint8_t MSG_TAG_TIME_LOCAL              = 0x12; // 18
const uint8_t MSG_TAG_INSTRUMENTATION         = 0x13; // 19
const uint8_t MSG_TAG_TIMER                   = 0x15; // 21
const uint8_t MSG_TAG_PILOT_CMD_ACK           = 0x20; // 32
const uint8_t MSG_TAG_VEHICLE_ARMED           = 0x21; // 33
const uint8_t MSG_TAG_PILOT_COMMAND           = 0x22; // 34
const uint8_t MSG_TAG_SBUS_ACK                = 0x23; // 35
const uint8_t MSG_TAG_VOTING_STATUS           = 0x24; // 36
const uint8_t MSG_TAG_STREAM_STATUS           = 0x25; // 37
const uint8_t MSG_TAG_INTERNAL_STATES         = 0x2A; // 42
const uint8_t MSG_TAG_ANGULAR_RATES           = 0x30; // 48
const uint8_t MSG_TAG_ATTITUDE_QUAT           = 0x31; // 49
const uint8_t MSG_TAG_RATES_SETPOINT          = 0x32; // 50
const uint8_t MSG_TAG_ATTITUDE_EULER          = 0x33; // 51
const uint8_t MSG_TAG_CAS_REF                 = 0x34; // 52
const uint8_t MSG_TAG_POSITION_NED            = 0x35; // 53
const uint8_t MSG_TAG_VELOCITY_DOWN           = 0x37; // 55
const uint8_t MSG_TAG_VELOCITY_DOWN_REF       = 0x38; // 56
const uint8_t MSG_TAG_RATE_PIDS               = 0x39; // 57
const uint8_t MSG_TAG_ANG_ACCEL_REF           = 0x3A; // 58
const uint8_t MSG_TAG_HEADING_VELOCITY        = 0x3B; // 59
const uint8_t MSG_TAG_UVC_REF                 = 0x3C; // 60
const uint8_t MSG_TAG_CAS_INTEGRATOR_STATES   = 0x3D; // 61
const uint8_t MSG_TAG_UVC_INTEGRATOR_STATES   = 0x3E; // 62
const uint8_t MSG_TAG_HEADING_ACCEL_REF       = 0x3F; // 63
const uint8_t MSG_TAG_ACTUATOR_CMD            = 0x40; // 64
const uint8_t MSG_TAG_GPS_HPPOSLLH            = 0x50; // 80
const uint8_t MSG_TAG_CURRENT                 = 0x53; // 83
const uint8_t MSG_TAG_VOLTAGE                 = 0x54; // 84
const uint8_t MSG_TAG_TEMPERATURE             = 0x55; // 85
const uint8_t MSG_TAG_RADAR_ALT_USD1          = 0x58; // 88
const uint8_t MSG_TAG_ACCELEROMETER           = 0x59; // 89
const uint8_t MSG_TAG_MAGNETOMETER            = 0x5A; // 90
const uint8_t MSG_TAG_LIDAR_TFMP              = 0x5B; // 91
const uint8_t MSG_TAG_PITOT_TUBE              = 0x5C; // 92
const uint8_t MSG_TAG_AIRDATA                 = 0x5D; // 93  
const uint8_t MSG_TAG_AHRS                    = 0x5F; // 95
const uint8_t MSG_TAG_IMU                     = 0x60; // 96
const uint8_t MSG_TAG_DEVICE_SENSE            = 0x61; // 97
const uint8_t MSG_TAG_SIM_STATE               = 0x73; // 115
const uint8_t MSG_TAG_DEBUG_VALUES        		= 0x7A; // 122

const uint16_t UPDATE_START_CODE = 0x1FF0;

const uint8_t PILOT_COMMAND_THROTTLE    = 0x01; // 1
const uint8_t PILOT_COMMAND_LAT_STICK   = 0x11; // 17
const uint8_t PILOT_COMMAND_LONG_STICK  = 0x12; // 18
const uint8_t PILOT_COMMAND_PEDAL       = 0x13; // 19
const uint8_t PILOT_COMMAND_WING_TILT   = 0x21; // 33
const uint8_t PILOT_COMMAND_ARM         = 0x31; // 49
const uint8_t PILOT_COMMAND_FLIGHT_MODE = 0x34; // 52
const uint8_t PILOT_COMMAND_MODE_2      = 0x37; // 55
const uint8_t PILOT_COMMAND_PARAM_1     = 0x41; // 65
const uint8_t PILOT_COMMAND_PARAM_2     = 0x42; // 66

const uint8_t ACTUATOR_WING_TILT_ABS    = 0x01; // 1
const uint8_t ACTUATOR_WING_TILT_REL    = 0x02; // 2
const uint8_t ACTUATOR_PROP_LEFT_1      = 0x11; // 17
const uint8_t ACTUATOR_PROP_LEFT_2      = 0x15; // 21
const uint8_t ACTUATOR_PROP_RIGHT_1     = 0x21; // 33
const uint8_t ACTUATOR_PROP_RIGHT_2     = 0x25; // 37
const uint8_t ACTUATOR_PROP_PITCH_DOWN  = 0x31; // 49
const uint8_t ACTUATOR_PROP_PITCH_UP    = 0x39; // 57
const uint8_t ACTUATOR_FLAPERON_LEFT_1  = 0x41; // 65
const uint8_t ACTUATOR_FLAPERON_LEFT_2  = 0x45; // 69
const uint8_t ACTUATOR_FLAPERON_RIGHT_1 = 0x51; // 81
const uint8_t ACTUATOR_FLAPERON_RIGHT_2 = 0x55; // 85
const uint8_t ACTUATOR_ELEVATOR_LEFT    = 0x61; // 97
const uint8_t ACTUATOR_ELEVATOR_RIGHT   = 0x65; // 101
const uint8_t ACTUATOR_RUDDER           = 0x71; // 113

/* DEPRECATED ---
const uint8_t ACTUATOR_WING_TILT_ABS          = 0x01;  // 1
const uint8_t ACTUATOR_WING_TILT_REL          = 0x02;  // 2
const uint8_t ACTUATOR_PROP_RIGHT_1           = 0x21;  // 33
const uint8_t ACTUATOR_PROP_TAIL_1            = 0x31;  // 49
const uint8_t ACTUATOR_PROP_TAIL_UP           = 0x39;  // 57
const uint8_t ACTUATOR_FLAPERON_LEFT_1        = 0x41;  // 65
const uint8_t ACTUATOR_FLAPERON_RIGHT_1       = 0x51;  // 81
const uint8_t ACTUATOR_STABILATOR             = 0x61;  // 97
const uint8_t ACTUATOR_RUDDER                 = 0x71;  // 113
*/

const uint8_t IMU_FLAG_GYRO_PRESENT  = 0x01;
const uint8_t IMU_FLAG_ACCEL_PRESENT = 0x02;
const uint8_t IMU_FLAG_BARO_PRESENT  = 0x04;

const uint8_t ADC_PRIMARY_FLAG   = 0x01;
const uint8_t ADC_SECONDARY_FLAG = 0x02;

const uint8_t STATE_AIR_GND_SWITCH = 0x01;

// Device addresses for device_sense messages
const uint8_t DEVSENSE_LINAK_PRESENT = 0x8B;
const uint8_t DEVSENSE_WINGTILT_L = 0x0B;
const uint8_t DEVSENSE_WINGTILT_R = 0x0C;

const uint8_t TIMER_ID_WORKING       = 0x01;
const uint8_t TIMER_ID_LOOP_INTERVAL = 0x02;

#pragma pack(push, 1)

struct sequence_msg
{
    uint8_t version;
    uint8_t sequence;
};

struct debug_msg
{
	uint8_t channel;
	float value;
};

#pragma pack(pop)

class SerialWriter
{
  private:
    StreamInterface *stream;

  private:
    uint8_t sequence_number = 0;

  public:
    SerialWriter(StreamInterface *stream);
  public:
    void write_message(uint8_t tag, uint8_t length, const void *value);
  public:
    void write_sequence_message();

  public:
    template<class T> void write_message(uint8_t tag, const T &value)
    {
      write_message(tag, sizeof(value), &value);
    }
};

} // namespace telem
