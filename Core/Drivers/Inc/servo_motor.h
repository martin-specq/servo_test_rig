/*
 * servo_motor.h
 *
 *  Created on: Feb 8, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SERVO_MOTOR_H_
#define DRIVERS_INC_SERVO_MOTOR_H_

struct ServoMotor
{
  float 		_angle_min_deg;
  float 		_angle_max_deg;
  float 		_pulse_width_min_us;
  float 		_pulse_width_max_us;
};

#endif /* DRIVERS_INC_SERVO_MOTOR_H_ */
