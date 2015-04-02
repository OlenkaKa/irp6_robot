// Copyright WUT 2014
#ifndef Irp6otmForwardKinematic_H_
#define Irp6otmForwardKinematic_H_

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <geometry_msgs/Pose.h>

#include <Eigen/Dense>
#include "Irp6otmKinematic.h"

class Irp6otmForwardKinematic : public RTT::TaskContext {
 public:
  Irp6otmForwardKinematic(const std::string& name);
  virtual ~Irp6otmForwardKinematic();

  bool configureHook();
  void updateHook();

 private:
  void direct_kinematics_transform(
      const Eigen::VectorXd& local_current_joints,
      Eigen::Affine3d* local_current_end_effector_frame);

  RTT::InputPort<Eigen::VectorXd> port_joint_position_;

  RTT::InputPort<geometry_msgs::Pose> port_tool_;
  RTT::OutputPort<geometry_msgs::Pose> port_output_wrist_pose_;
  RTT::OutputPort<geometry_msgs::Pose> port_output_end_effector_pose_;

  Eigen::VectorXd joint_position_;
  Eigen::VectorXd local_current_joints_tmp_;

  geometry_msgs::Pose tool_msgs_;

  //! D-H kinematic parameters - length of 1st segment.
  double d1;

  //! D-H kinematic parameters - length of 2nd segment.
  double a2;

  //! D-H kinematic parameters - length of 3rd segment.
  double a3;

  //! D-H kinematic parameters - length of 4th segment.
  double d5;

};

#endif /* Irp6otmForwardKinematic_H_ */
