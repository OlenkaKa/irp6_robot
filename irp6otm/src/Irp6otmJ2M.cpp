#include <rtt/Component.hpp>

#include "Irp6otmJ2M.h"
#include "Irp6otmTransmission.h"

Irp6otmJ2M::Irp6otmJ2M(const std::string& name)
    : RTT::TaskContext(name, PreOperational) {

  this->ports()->addPort("MotorPosition", port_motor_position_);
  this->ports()->addPort("JointPosition", port_joint_position_);
}

Irp6otmJ2M::~Irp6otmJ2M() {

}

bool Irp6otmJ2M::configureHook() {
  motor_position_.resize(NUMBER_OF_SERVOS);
  joint_position_.resize(NUMBER_OF_SERVOS);
  return true;
}

void Irp6otmJ2M::updateHook() {
  if (port_joint_position_.read(joint_position_) == RTT::NewData) {
    if (i2mp(&joint_position_(0), &motor_position_(0))) {
      port_motor_position_.write(motor_position_);
    }
  }
}

bool Irp6otmJ2M::i2mp(const double* joints, double* motors) {

  // Niejednoznacznosc polozenia dla 4-tej osi (obrot kisci < 180).
  const double joint_4_revolution = M_PI;
  // Niejednoznacznosc polozenia dla 5-tej osi (obrot kisci > 360).
  const double axis_5_revolution = 2 * M_PI;

  // Obliczanie kata obrotu walu silnika napedowego toru
  motors[0] = GEAR[0] * joints[0] + SYNCHRO_JOINT_POSITION[0];

  // Obliczanie kata obrotu walu silnika napedowego kolumny
  motors[1] = GEAR[1] * joints[1] + SYNCHRO_JOINT_POSITION[1];

  // Obliczanie kata obrotu walu silnika napedowego ramienia dolnego
  motors[2] = GEAR[2]
      * sqrt(sl123 + mi2 * cos(joints[2]) + ni2 * sin(-joints[2]))
      + SYNCHRO_JOINT_POSITION[2];

  // Obliczanie kata obrotu walu silnika napedowego ramienia gornego
  motors[3] = GEAR[3]
      * sqrt(sl123 + mi3 * cos(joints[3] + joints[2] + M_PI_2) + ni3 * sin(-(joints[3] + joints[2] + M_PI_2)))
      + SYNCHRO_JOINT_POSITION[3];

  // Obliczanie kata obrotu walu silnika napedowego obotu kisci T
  // jesli jest mniejsze od -pi/2

  double joints_tmp4 = joints[4] + joints[3] + joints[2] + M_PI_2;
  if (joints_tmp4 < -M_PI_2)
    joints_tmp4 += joint_4_revolution;

  motors[4] = GEAR[4] * (joints_tmp4 + THETA[4]) + SYNCHRO_JOINT_POSITION[4];

  // Obliczanie kata obrotu walu silnika napedowego obrotu kisci V
  motors[5] = GEAR[5] * joints[5] + SYNCHRO_JOINT_POSITION[5] + motors[4];

  // Ograniczenie na obrot.
  while (motors[5] < LOWER_MOTOR_LIMIT[5])
    motors[5] += axis_5_revolution;
  while (motors[5] > UPPER_MOTOR_LIMIT[5])
    motors[5] -= axis_5_revolution;

  // Obliczanie kata obrotu walu silnika napedowego obrotu kisci N
  motors[6] = GEAR[6] * joints[6] + SYNCHRO_JOINT_POSITION[6];

  return checkMotorPosition(motors);
}

bool Irp6otmJ2M::checkMotorPosition(const double * motor_position) {

  if (motor_position[0] < LOWER_MOTOR_LIMIT[0])  // Kat f1 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_0);
  else if (motor_position[0] > UPPER_MOTOR_LIMIT[0])  // Kat f1 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_0);

  if (motor_position[1] < LOWER_MOTOR_LIMIT[1])  // Kat f2 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_1);
  else if (motor_position[1] > UPPER_MOTOR_LIMIT[1])  // Kat f2 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_1);

  if (motor_position[2] < LOWER_MOTOR_LIMIT[2])  // Kat f3 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_2);
  else if (motor_position[2] > UPPER_MOTOR_LIMIT[2])  // Kat f3 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_2);

  if (motor_position[3] < LOWER_MOTOR_LIMIT[3])  // Kat f4 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_3);
  else if (motor_position[3] > UPPER_MOTOR_LIMIT[3])  // Kat f4 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_3);

  if (motor_position[4] < LOWER_MOTOR_LIMIT[4])  // Kat f5 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_4);
  else if (motor_position[4] > UPPER_MOTOR_LIMIT[4])  // Kat f5 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_4);

  if (motor_position[5] < LOWER_MOTOR_LIMIT[5])  // Kat f6 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_5);
  else if (motor_position[5] > UPPER_MOTOR_LIMIT[5])  // Kat f6 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_5);

  if (motor_position[6] < LOWER_MOTOR_LIMIT[6])  // Kat f6 mniejszy od minimalnego
    return false;  //throw NonFatal_error_2(BEYOND_LOWER_LIMIT_AXIS_6);
  else if (motor_position[6] > UPPER_MOTOR_LIMIT[6])  // Kat f6 wiekszy od maksymalnego
    return false;  //throw NonFatal_error_2(BEYOND_UPPER_LIMIT_AXIS_6);

  return true;
}  //: check_motor_position

ORO_CREATE_COMPONENT(Irp6otmJ2M)

