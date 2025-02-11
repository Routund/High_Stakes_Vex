#include "vex.h"

using namespace vex;

// A global instance of competition

competition Competition;
controller Cotton_candy;
brain pancakes;

motor LFdrive(PORT16, ratio18_1);
motor RFdrive(PORT20, ratio18_1, true);
motor LBdrive(PORT1, ratio18_1);
motor RBdrive(PORT12, ratio18_1, true);
motor TopChainMotor(PORT12, ratio18_1, true);
motor FlexMotor(PORT16, ratio18_1, true);

digital_out pistonPort(pancakes.ThreeWirePort.H);

motor_group LeftSide = motor_group (LFdrive, LBdrive);
motor_group RightSide = motor_group (RFdrive, RBdrive);



bool pistonDown = false;
bool armDown = false;
bool flex_on = false;
bool top_chain_on = false;
int intake_mode = 0;

// define your global instances of motors and other devices here

float lateral_position_offset = 0;
float rotation_position_offset = 0;

float lateral_movement = 0;
float rotation_desired = 0;

bool drive_pd_enabled = false;

float l_diff_in_position = 0;
float l_previous_diff = 0;
float l_speed = 0;

float r_diff_in_position = 0;
float r_previous_diff = 0;
float r_speed = 0;

void drive_pd() {
  if (drive_pd_enabled){

    float average_lateral_position = (LeftSide.position(degrees) + RightSide.position(degrees))/2 - lateral_position_offset;
    l_diff_in_position = average_lateral_position - lateral_movement;
    l_speed = l_diff_in_position - l_previous_diff;

    float average_rotational_position = (LeftSide.position(degrees) - RightSide.position(degrees))/2 - rotation_position_offset;
    r_diff_in_position = average_rotational_position - rotation_desired;
    r_speed = r_diff_in_position - r_previous_diff;
    
    float lateral_motor_power = (l_diff_in_position * 0.001 + l_speed * 0.0001)/12;
    float rotation_motor_power = (r_diff_in_position * 0.001 + r_speed * 0.0001)/12;
    LeftSide.spin(forward,lateral_motor_power + rotation_motor_power,voltageUnits::volt);
    RightSide.spin(forward,lateral_motor_power + rotation_motor_power,voltageUnits::volt);
  }
  this_thread::sleep_for(20);
}

void driveForward(float distance) {

        // float inchesPerDegree = 25;
        // float timer = 0;       

        // while (timer < 30){
        // LeftSide.spin(vex::forward, inchesPerDegree * distance, vex::percent);
        // RightSide.spin(vex::forward, inchesPerDegree * distance, vex::percent);
        // timer++;
        // }

        // wait(distance2,msec);
        // LeftSide.stop();
        // RightSide.stop();
        // RightSide.spin(vex::forward, 0, vex::percent);
        // LeftSide.spin(vex::forward,0, vex::percent);

        lateral_position_offset = (LeftSide.position(degrees) + RightSide.position(degrees))/2;
        lateral_movement = distance;
    }

void rotate(float angle){

  // if (angle>0){
  //     LeftSide.spin(vex::forward, 25, vex::percent);
  //     RightSide.spin(vex::forward, -25 , vex::percent);

  //     wait(angle*10.8,msec);

  //     LeftSide.stop();
  //     RightSide.stop();
  //     RightSide.spin(vex::forward, 0, vex::percent);
  //     LeftSide.spin(vex::forward,0, vex::percent);
  // }

  // else{
  //     LeftSide.spin(vex::forward, -25, vex::percent);
  //     RightSide.spin(vex::forward, 25 , vex::percent);

  //     wait(angle*-10.8,msec);

  //     LeftSide.stop();
  //     RightSide.stop();
  //     RightSide.spin(vex::forward, 0, vex::percent);
  //     LeftSide.spin(vex::forward,0, vex::percent);
  // }

  rotation_position_offset = (LeftSide.position(degrees) - RightSide.position(degrees))/2;
  rotation_desired = angle * 8;
}

void PistonToggle(){
    pistonPort.set(!pistonDown);
    pistonDown= !pistonDown; 
}

void intake_system_forward(){
  if (intake_mode!=1){
    top_chain_on = true;
    flex_on = true;
    TopChainMotor.spin(reverse,180.0,vex::velocityUnits::dps);
    FlexMotor.spin(forward,220.0,vex::velocityUnits::dps);
    intake_mode = 1;
  }
  else{
    top_chain_on = false;
    flex_on = false;
    TopChainMotor.stop();
    FlexMotor.stop();
    intake_mode = 0;
  }
}

void intake_system_reverse(){
  if (intake_mode!=-1){
    top_chain_on = true;
    flex_on = true;
    TopChainMotor.spin(reverse,-180.0,vex::velocityUnits::dps);
    FlexMotor.spin(forward,-220.0,vex::velocityUnits::dps);
    intake_mode = -1;
  }
  else{
    top_chain_on = false;
    flex_on = false;
    TopChainMotor.stop();
    FlexMotor.stop();
    intake_mode = 0;
  }
}

void ChainForward(float fraction){
    TopChainMotor.spin(forward,110.0,vex::velocityUnits::dps);
    wait(2500*fraction,msec);
    TopChainMotor.stop();
}

void ChainReverse(float fraction){
    TopChainMotor.spin(reverse,350.0,vex::velocityUnits::dps);
    wait(1500*fraction,msec);
    TopChainMotor.stop();
}

void flex_forward_toggle(){
  flex_on = !flex_on;
  if (flex_on){
    FlexMotor.spin(reverse,220.0,vex::velocityUnits::dps);
  }
  else{
    FlexMotor.stop();
  }
}

void top_chain_forward_toggle(){
  top_chain_on = !top_chain_on;
  if (flex_on){
    TopChainMotor.spin(forward,180.0,vex::velocityUnits::dps);
  }
  else{
    TopChainMotor.stop();
  }
}

void pre_auton(void) {
  Cotton_candy.ButtonA.pressed(PistonToggle);

  pistonPort.set(false);

  TopChainMotor.setStopping(brakeType::brake);
  FlexMotor.setStopping(brakeType::brake);

  LeftSide.resetPosition();
  RightSide.resetPosition();
}



void autonomous(void) {

  drive_pd_enabled = true;
  vex::thread(drive_pd); 

  driveForward(360);

  wait(2000,msec);

  top_chain_forward_toggle();
  wait(1000,msec);
  
  driveForward(-360);

}

void usercontrol(void) {

    // User control code here, inside the loop
    //runs the loop over and over throughout the competition

    while (1) {
    drive_pd_enabled = false;

    int leftSpin = Cotton_candy.Axis3.position(percent);
    int rightSpin = Cotton_candy.Axis2.position(percent);

    if(leftSpin==0){
      LeftSide.setStopping(brakeType::brake);
    }

    if (rightSpin==0)
    {
      RightSide.setStopping(brakeType::brake);
    }

    LeftSide.spin(vex::forward, leftSpin, vex::percent);
    RightSide.spin(vex::forward, rightSpin, vex::percent);

    if (Cotton_candy.ButtonR1.pressing()){
      TopChainMotor.spin(forward,180.0,vex::velocityUnits::dps);
    }
    else if (Cotton_candy.ButtonL1.pressing()){
      TopChainMotor.spin(reverse,180.0,vex::velocityUnits::dps);
    }
    else {
      TopChainMotor.stop();
    }

  wait(20, msec); // Sleep the task for a short amount of time to prevent wasted resources.
  }
}

// Main will set up the competition functions and callbacks.

int main() {

  // Set up callbacks for autonomous and driver control periods.

  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  Cotton_candy.ButtonLeft.pressed(flex_forward_toggle);
  Cotton_candy.ButtonRight.pressed(top_chain_forward_toggle);
  Cotton_candy.ButtonDown.pressed(intake_system_reverse);
  Cotton_candy.ButtonUp.pressed(intake_system_forward);

  // Run the pre-autonomous function.

  pre_auton();

  // Prevent main from exiting with an infinite loop.

  while (true) {
  wait(100, msec);
  }
}
