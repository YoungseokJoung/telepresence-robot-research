#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <iostream>
#include <string>
using namespace std;

/*
bool startswith(std::vector<std::string> mainStr, std::string toMatch)
{
        // std::string::find returns 0 if toMatch is found at starting
        if(mainStr.find(toMatch) == 0)
                return true;
        else
                return false;
}
*/

int main(int argc, char** argv){

  ros::init(argc, argv, "my_tf_listener");
  ros::NodeHandle nh("~");

  tf::TransformListener listener;

  ros::Rate rate(10.0);

  std::string fixed_frame;
  std::vector<std::string> f;
  std::vector<std::string> h;

  bool skeleton_detected = false;
  int i = 0;
  nh.param<std::string>("fixed_frame", fixed_frame, "openni_depth_frame");

  // Make sure we see the openni_depth_frame
  ros::Time now = ros::Time::now();
  listener.waitForTransform(fixed_frame, fixed_frame, now, ros::Duration(30.0));

  while (nh.ok()){
    tf::StampedTransform transform;
    printf("main loop start\n");
    while(!skeleton_detected){
	printf("not detected yet\n");
	listener.getFrameStrings(f);
/*	for(i = 0; i<100; i++){
		printf("%c",f[i]);
	}

        for(i = 0; i<5; i++){
                printf("head? => %c",h[i]);
        }

        if(startswith(f, "head_")){
		printf("work\n");
        }*/

     try{
        listener.lookupTransform(fixed_frame, fixed_frame, ros::Time(0), transform);
        //std::cout<<transform::child_frame_id<<"\n";
      }catch (tf::TransformException ex){
      ROS_ERROR("%s",ex.what());
      }

    }

    rate.sleep();
  }
  return 0;
};


