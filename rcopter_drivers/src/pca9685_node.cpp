#include <ros/ros.h>
#include <mavros_msgs/RCOut.h>

#ifdef HAS_BCM2835
  #include <rcopter_drivers/pca9685.hpp>
  PCA9685 pca9685;
#else
  #include <rcopter_drivers/mock_pca9685.hpp>
  MockPCA9685 pca9685;
#endif

bool need_pwm_update = true;
float pwm_pulse_width[4] = {0, 0, 0, 0};

void rcOutCB(mavros_msgs::RCOut::ConstPtr msg){
  for(int i = 0; i < 4; i++){
    pwm_pulse_width[i] = msg->channels[i];
  }
  need_pwm_update = true;
}
int main(int argc, char* argv[]){
  ros::init(argc, argv, "pca9685_node");
  ros::NodeHandle nh;
  ros::Subscriber sub = nh.subscribe("rc_out", 1, rcOutCB);
  ros::Rate rate(100);
  
  pca9685.initialize();
  pca9685.setFrequency(50);
  
  while(ros::ok()){
    if(need_pwm_update){
      for(int i = 0; i < 4; i++){
	pca9685.setPWM(i+3, 0, pwm_pulse_width[i]);
      }
      need_pwm_update = false;
    }
    ros::spinOnce();
    rate.sleep();
  }
  pca9685.finalize();
  return 0;
}
