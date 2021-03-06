#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <std_msgs/Float64.h>
#include <std_msgs/Int64.h>
#include <rtt/Component.hpp>
#include <std_srvs/Empty.h>
#include <ros/ros.h>

#include "SarkofagRegulator.h"

const int MAX_PWM = 190;

SarkofagRegulator::SarkofagRegulator(const std::string& name)
    : TaskContext(name),
      posInc_in("posInc_in"),
      deltaInc_in("deltaInc_in"),
      computedPwm_out("computedPwm_out"),
      a_(0.0),
      b0_(0.0),
      b1_(0.0),
      delta_eint_old(0.0),
      delta_eint(0.0),
      deltaIncData(0.0),
      output_value(0.0),
      posIncData(0.0),
      position_increment_new(0.0),
      position_increment_old(0.0),
      set_value_new(0.0),
      set_value_old(0.0),
      set_value_very_old(0.0),
      step_new(0.0),
      step_old(0.0),
      step_old_pulse(0.0)
      {

  this->addEventPort(posInc_in).doc("Receiving a value of position step");
  this->addPort(deltaInc_in).doc("Receiving a value of measured increment.");
  this->addPort(computedPwm_out).doc("Sending value of calculated pwm.");

  this->addProperty("A", A_).doc("");
  this->addProperty("BB0", BB0_).doc("");
  this->addProperty("BB1", BB1_).doc("");
  this->addProperty("current_mode", current_mode_).doc("");
  this->addProperty("max_output_current", max_output_current_).doc("");
  this->addProperty("current_reg_kp", current_reg_kp_).doc("");
  this->addProperty("reg_number", reg_number_).doc("");
  this->addProperty("debug", debug_).doc("");
  this->addProperty("eint_dif", eint_dif_).doc("");

  step_reg=0;
}

SarkofagRegulator::~SarkofagRegulator() {

}

bool SarkofagRegulator::configureHook() {
  reset();

  a_ = A_;
  b0_ = BB0_;
  b1_ = BB1_;

  return true;
}

void SarkofagRegulator::updateHook() {
  if (NewData == posInc_in.read(posIncData)
      && NewData == deltaInc_in.read(deltaIncData)) {
    computedPwm_out.write(doServo(posIncData, deltaIncData));
  }
}

int SarkofagRegulator::doServo(double step_new, int pos_inc) {

  // algorytm regulacji dla serwomechanizmu
  // position_increment_old - przedostatnio odczytany przyrost polozenie
  //                         (delta y[k-2] -- mierzone w impulsach)
  // position_increment_new - ostatnio odczytany przyrost polozenie
  //                         (delta y[k-1] -- mierzone w impulsach)
  // step_old_pulse               - poprzednia wartosc zadana dla jednego kroku
  //                         regulacji (przyrost wartosci zadanej polozenia --
  //                         delta r[k-2] -- mierzone w impulsach)
  // step_new               - nastepna wartosc zadana dla jednego kroku
  //                         regulacji (przyrost wartosci zadanej polozenia --
  //                         delta r[k-1] -- mierzone w radianach)
  // set_value_new          - wielkosc kroku do realizacji przez HIP
  //                         (wypelnienie PWM -- u[k]): czas trwania jedynki
  // set_value_old          - wielkosc kroku do realizacji przez HIP
  //                         (wypelnienie PWM -- u[k-1]): czas trwania jedynki
  // set_value_very_old     - wielkosc kroku do realizacji przez HIP
  //                         (wypelnienie PWM -- u[k-2]): czas trwania jedynki

  double step_new_pulse;  // nastepna wartosc zadana dla jednego kroku regulacji

  step_new_pulse = step_new;
  position_increment_new = pos_inc;

  // Przyrost calki uchybu
  delta_eint = delta_eint_old
      + (1.0 + eint_dif_) * (step_new_pulse - position_increment_new)
      - (1.0 - eint_dif_) * (step_old_pulse - position_increment_old);

  //std::cout << "POS INCREMENT NEW: " << position_increment_new <<  std::endl;

  // obliczenie nowej wartosci wypelnienia PWM algorytm PD + I
  set_value_new = (1 + a_) * set_value_old - a_ * set_value_very_old
      + b0_ * delta_eint - b1_ * delta_eint_old;


  //std::cout << "PWM VALUE (" << pos_inc << " to " << pos_inc_new << ") IS " << (int)set_value_new << std::endl;



  // ograniczenie na sterowanie
  if (set_value_new > MAX_PWM)
    set_value_new = MAX_PWM;
  if (set_value_new < -MAX_PWM)
    set_value_new = -MAX_PWM;

  if (current_mode_) {
    output_value = set_value_new * current_reg_kp_;
    if (output_value > max_output_current_) {
      output_value = max_output_current_;
    } else if (output_value < -max_output_current_) {
      output_value = -max_output_current_;
    }
  } else {
    output_value = set_value_new;
  }

  	step_reg++;

  if (debug_) {
    std::cout << step_reg << "  output_value: " << output_value << "  step_new: " << step_new << "  pos_inc: " << pos_inc << std::endl;
  }

  // przepisanie nowych wartosci zmiennych do zmiennych przechowujacych wartosci poprzednie
  position_increment_old = position_increment_new;
  delta_eint_old = delta_eint;
  step_old_pulse = step_new_pulse;
  set_value_very_old = set_value_old;
  set_value_old = set_value_new;

  return ((int) output_value);
}

void SarkofagRegulator::reset() {
  position_increment_old = 0.0;
  position_increment_new = 0.0;
  step_old_pulse = 0.0;
  step_new = 0.0;
  set_value_new = 0.0;
  set_value_old = 0.0;
  set_value_very_old = 0.0;
  delta_eint = 0.0;
  delta_eint_old = 0.0;
}

ORO_CREATE_COMPONENT(SarkofagRegulator)
