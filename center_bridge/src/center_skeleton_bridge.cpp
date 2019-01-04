// subscribe from center and skeleton tracking + client requesting to robot.

#include "center_bridge/cmd_from_hive.h"
#include "ros/ros.h"
#include <sstream>

// file
#include <fstream>
#include <visualization_msgs/Marker.h>

// creating directory
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//socket
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

//arm calculation
#include <cmath>
//#include <float.h>

#define PORT 3170

using namespace std;

//cmd from hive
string cmd;
int val;

//arm variable
float left_arm_angle[3];
float prevJoint5angle;

char lelbow_char[20];

// open file and write headers
void open_data_file(ofstream& myfile, string filename) {
	myfile.open (filename.c_str());
	myfile << "neck_x" << ",";
	myfile << "neck_y" << ",";
	myfile << "neck_z" << ",";
	myfile << "torso_x" << ",";
	myfile << "torso_y" << ",";
	myfile << "torso_z" << ",";
	myfile << "lshoulder_x" << ",";
	myfile << "lshoulder_y" << ",";
	myfile << "lshoulder_z" << ",";
	myfile << "lelbow_x" << ",";
	myfile << "lelbow_y" << ",";
	myfile << "lelbow_z" << ",";
	myfile << "lwrist_x" << ",";
	myfile << "lwrist_y" << ",";
	myfile << "lwrist_z" << ",";
	myfile << "Joint5" << ",";
	myfile << "Joint6" << ",";
	myfile << "Joint7" << ",";

	myfile << endl;
}

void save_data_file(ofstream& myfile, 
	const visualization_msgs::Marker marker, 
	float left_arm_angle[]) {
	
	myfile << marker.points[1].x << ",";
	myfile << marker.points[1].y << ",";
	myfile << marker.points[1].z << ",";
	myfile << marker.points[2].x << ",";
	myfile << marker.points[2].y << ",";
	myfile << marker.points[2].z << ",";
	myfile << marker.points[3].x << ",";
	myfile << marker.points[3].y << ",";
	myfile << marker.points[3].z << ",";
	myfile << marker.points[4].x << ",";
	myfile << marker.points[4].y << ",";
	myfile << marker.points[4].z << ",";
	myfile << marker.points[5].x << ",";
	myfile << marker.points[5].y << ",";
	myfile << marker.points[5].z << ",";

	myfile << left_arm_angle[0] << ",";
	myfile << left_arm_angle[1] << ",";
	myfile << left_arm_angle[2] << ",";

	myfile << endl;
}


void skeletonCb(visualization_msgs::Marker marker){

	float neck[3];
	float torso[3];
	float lshoulder[3];
	float lelbow[3];
	float lwrist[3];

	float vlel2lsh[3];
	float vlel2lwr[3];
	float vlelproj2xz[3];
	float vlwrproj2xz[3];
	float vlelproj2yz[3];
	float vnec2tor[3];

	float v1[3];
	float v2[3];
	float v1norm[3];
	float v2norm[3];

	float mag;
	float dot_product;
	float tmp_angle;

	static ofstream data_file;
	static bool data_file_opened = false;
	static bool save_data = true;
	struct stat st = {0};

	//3D position from skeleton tracking
    neck[0] = marker.points[1].z;
    neck[1] = marker.points[1].z;
    neck[2] = marker.points[1].z;	// nec location
    torso[0] = marker.points[2].z;
    torso[1] = marker.points[2].z;
    torso[2] = marker.points[2].z;	// torso location
	lshoulder[0] = marker.points[3].x;
    lshoulder[1] = marker.points[3].y;
    lshoulder[2] = marker.points[3].z;	// lshoulder location 
	lelbow[0] = marker.points[4].x;
    lelbow[1] = marker.points[4].y;
    lelbow[2] = marker.points[4].z;	// lelbow location
	lwrist[0] = marker.points[5].x;
    lwrist[1] = marker.points[5].y;
    lwrist[2] = marker.points[5].z;	// lwrist location 

    // vectors for angle calculation
	vlel2lsh[0] = lshoulder[0] - lelbow[0];
	vlel2lsh[1] = lshoulder[1] - lelbow[1];
	vlel2lsh[2] = lshoulder[2] - lelbow[2];
	vlel2lwr[0] = lwrist[0] - lelbow[0];
	vlel2lwr[1] = lwrist[1] - lelbow[1];
	vlel2lwr[2] = lwrist[2] - lelbow[2]; // vectors for Joint 6
	
	vlwrproj2xz[0] = lwrist[0] - lshoulder[0];
	vlwrproj2xz[1] = lshoulder[1] - lshoulder[1];
	vlwrproj2xz[2] = lwrist[2] - lshoulder[2];
	
	vlelproj2yz[0] = lshoulder[0] - lshoulder[0];
	vlelproj2yz[1] = lelbow[1] - lshoulder[1];
	vlelproj2yz[2] = lelbow[2] - lshoulder[2];
	
	vnec2tor[0] = torso[0] - neck[0];
	vnec2tor[1] = torso[1] - neck[1];
	vnec2tor[2] = torso[2] - neck[2]; // vectors for Joint 5,7

	// norm vector for Joint 6
	mag = sqrt(vlel2lwr[0] * vlel2lwr[0] + vlel2lwr[1] * vlel2lwr[1] + vlel2lwr[2] * vlel2lwr[2]);
	v1norm[0] = vlel2lwr[0] / mag;
	v1norm[1] = vlel2lwr[1] / mag;
	v1norm[2] = vlel2lwr[2] / mag;
	mag = sqrt(vlel2lsh[0] * vlel2lsh[0] + vlel2lsh[1] * vlel2lsh[1] + vlel2lsh[2] * vlel2lsh[2]);
	v2norm[0] = vlel2lsh[0] / mag;
	v2norm[1] = vlel2lsh[1] / mag;
	v2norm[2] = vlel2lsh[2] / mag;

	// calculate angle of left arm Joint 6
	dot_product = v1norm[0]*v2norm[0] + v1norm[1]*v2norm[1] + v1norm[2]*v2norm[2];
	tmp_angle = acos(dot_product);

	// callibrate and put angles
	left_arm_angle[1] = (tmp_angle * (58000) / (3.141592)) - 51000;

	// norm vector for Joint 7, 5
	mag = sqrt(vnec2tor[0] * vnec2tor[0] + vnec2tor[1] * vnec2tor[1] + vnec2tor[2] * vnec2tor[2]);
	v2norm[0] = vnec2tor[0] / mag;
	v2norm[1] = vnec2tor[1] / mag;
	v2norm[2] = vnec2tor[2] / mag;

	mag = sqrt(vlwrproj2xz[0] * vlwrproj2xz[0] + vlwrproj2xz[1] * vlwrproj2xz[1] + vlwrproj2xz[2] * vlwrproj2xz[2]);
	v1norm[0] = vlwrproj2xz[0] / mag;
	v1norm[1] = vlwrproj2xz[1] / mag;
	v1norm[2] = vlwrproj2xz[2] / mag;

	// calculate angle of left arm Joint 7
	dot_product = v1norm[0]*v2norm[0] + v1norm[1]*v2norm[1] + v1norm[2]*v2norm[2];
	tmp_angle = acos(dot_product);

	// callibrate and put angles
	left_arm_angle[2] = (tmp_angle * (-130000) / 3.141592) + 25000;

	// norm vector for Joint 5	
	mag = sqrt(vlelproj2yz[0] * vlelproj2yz[0] + vlelproj2yz[1] * vlelproj2yz[1] + vlelproj2yz[2] * vlelproj2yz[2]);
	v1norm[0] = vlelproj2yz[0] / mag;
	v1norm[1] = vlelproj2yz[1] / mag;
	v1norm[2] = vlelproj2yz[2] / mag;
	
	// calculate angle of left arm Joint 5
	dot_product = v1norm[0]*v2norm[0] + v1norm[1]*v2norm[1] + v1norm[2]*v2norm[2];
	tmp_angle = acos(dot_product);

 	// callibration and put angles
	if(mag < 0.05){
		left_arm_angle[0] = 5000;
	}else
		left_arm_angle[0] = (tmp_angle * (-60000) / (3.141592)) + 23000;
	
	left_arm_angle[1] = left_arm_angle[1] + left_arm_angle[0] - prevJoint5angle;

	prevJoint5angle = left_arm_angle[0];

	if(!data_file_opened){
		if (stat("/home/interactionlab/arm_data", &st) == -1) {
			mkdir("/home/interactionlab/arm_data", 0700);
		}
		open_data_file(data_file, "/home/interactionlab/arm_data/arm_data.txt");
		data_file_opened = true;
	}

	// save continuous data
	if(save_data){
		save_data_file(data_file, marker, left_arm_angle);
	}


}

void messageCb(center_bridge::cmd_from_hive cmd_from_hive){
	ROS_INFO("message_recieved : %s %d", cmd_from_hive.cmd.c_str(), cmd_from_hive.val);
	cmd = cmd_from_hive.cmd.c_str();
	val = cmd_from_hive.val;
}

int main(int argc, char **argv){
    ros::init(argc, argv, "center_skeleton_bridge");
    ros::NodeHandle nh;
 
    ros::Subscriber marker_sub = nh.subscribe<visualization_msgs::Marker>("skeleton_markers", 100, skeletonCb);
	ros::Subscriber center_sub = nh.subscribe("cmd_from_hive", 100, messageCb);
	ros::Rate loop_rate(10);

	// socket connection -> make function for this later
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	
	bool connection = false;

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	//cmd buffer
	char cmd2robot[257] = "";
	char from_robot[1024];

	//for torque value
	stringstream torque;

	//base light variable
	int light_value = 0;
	char tmp_str_char[20];
	std::string color_value;

	//skeleton_cordinates -> (need to be changed to angles)
	float lelbow_x;
	std::string angle_value;

	//stringstream 
	stringstream tmp_str;
	stringstream tmp_cmd;

	while(ros::ok()){
		if(cmd == "con2robot" && val == 0 && connection == true){

			//beginning position setup
			tmp_cmd << std::string("vstate 5000 -5000 0\n");
			strncpy(cmd2robot, tmp_cmd.str().c_str(), sizeof(cmd2robot));

			//send arm cmd to robot
			if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
				std::cout<<"send failed"<<endl;
			else{
			    printf("cmd = %s", cmd2robot);
				usleep(100000);
			}

			//clear buffer
			for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
			}
			for (int i = 0; i < sizeof(from_robot); i++) {
		        from_robot[i] = '\0';
			}

			tmp_str.str("");
			tmp_str.clear();
			tmp_cmd.str("");
			tmp_cmd.clear();
		
			//torque off
			for(int i = 5; i < 8; i++){
				//make torque cmd
			    torque << std::string("torque ") + std::to_string(i) + " off\n"; //c++11
		        strncpy(cmd2robot, torque.str().c_str(), sizeof(cmd2robot));

		        //send torque on cmd
				if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
					std::cout<<"send failed"<<endl;
		        else{
		            printf("cmd = %s", cmd2robot);
		            usleep(500000);
		        }
		        torque.str("");
		        torque.clear();
			}    

			//clear buffer
		    for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
		    }

			cout << "cmd = "<< cmd << endl;
			cout << "val = "<< val << endl;

			//disconnect client from robot
			close(sock);

			connection = false;
		}

		if(cmd == "con2robot" && val == 1 && connection == false){

			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("\n Socket creation error \n");
				return -1;
			}

			// Convert IPv4 and IPv6 addresses from text to binary form
			if(inet_pton(AF_INET, "127.0.0.1"/*"192.168.1.127"*/, &serv_addr.sin_addr)<=0) 
			{
				printf("\nInvalid address/ Address not supported \n");
				return -1;
			}

			if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
			{
				printf("\nConnection Failed \n");
				return -1;
			}		

			//torque on
			for(int i = 5; i < 8; i++){
				//make torque cmd
			    torque << std::string("torque ") + std::to_string(i) + " on\n"; //c++11
		        strncpy(cmd2robot, torque.str().c_str(), sizeof(cmd2robot));

		        //send torque on cmd
				if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
					std::cout<<"send failed"<<endl;
		        else{
		            printf("cmd = %s", cmd2robot);
		            usleep(500000);
		        }
		        torque.str("");
		        torque.clear();
			}    

			//clear buffer
		    for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
		    }

			//beginning position setup
			tmp_cmd << std::string("vstate 5000 -5000 0\n");
			strncpy(cmd2robot, tmp_cmd.str().c_str(), sizeof(cmd2robot));

			//send arm cmd to robot
			if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
				std::cout<<"send failed"<<endl;
			else{
			    printf("cmd = %s", cmd2robot);
				usleep(100000);
			}
			//clear buffer
			for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
			}
			for (int i = 0; i < sizeof(from_robot); i++) {
		        from_robot[i] = '\0';
			}

			tmp_str.str("");
			tmp_str.clear();
			tmp_cmd.str("");
			tmp_cmd.clear();

			connection = true;

		}
		
		if(cmd == "automode" && val == 1){

			//check robot response for previous cmd
			if(recv(sock, from_robot, sizeof(from_robot)/sizeof(*from_robot), 0) < 0)
				std::cout<<"recv failed";
			else{
			    printf("from robot : %s", from_robot);
			}

	        //make light cmd
			for(int i = 0; i < 3; i++){
				light_value = rand() % 255 + 0;
				tmp_str<<light_value;
				color_value = color_value + " " + tmp_str.str();  // tmp_str_char not necessary.
				tmp_str.str("");
	            tmp_str.clear();
			}

			//put cmd into buffer
	        tmp_cmd << std::string("light_color 20") + color_value + "\n";
	        strncpy(cmd2robot, tmp_cmd.str().c_str(), sizeof(cmd2robot));
	        color_value.clear();

	        //send light cmd to robot
	        if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
				std::cout<<"send failed"<<endl;
			else{
			    printf("cmd = %s", cmd2robot);
				usleep(300000);
			}

			//clear buffer
			for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
			}

			tmp_str.str("");
			tmp_str.clear();
			tmp_cmd.str("");
			tmp_cmd.clear();

			//make arm angle cmd
	    	for(int i = 0; i < 3; i++){
				tmp_str<<left_arm_angle[i];
				angle_value = angle_value + " " + tmp_str.str();
				tmp_str.str("");
				tmp_str.clear();
			}

			tmp_cmd << std::string("vstate") + angle_value + "\n";
			strncpy(cmd2robot, tmp_cmd.str().c_str(), sizeof(cmd2robot));
	        angle_value.clear();

	        //send arm cmd to robot
	        if(send(sock, cmd2robot, strlen(cmd2robot), 0) < 0)
				std::cout<<"send failed"<<endl;
			else{
			    printf("cmd = %s", cmd2robot);
				usleep(300000);
			}

			//clear buffer
			for (int i = 0; i < sizeof(cmd2robot); i++) {
		        cmd2robot[i] = '\0';
			}
			for (int i = 0; i < sizeof(from_robot); i++) {
		        from_robot[i] = '\0';
			}

			tmp_str.str("");
			tmp_str.clear();
			tmp_cmd.str("");
			tmp_cmd.clear();
		}

		// send cmd to raspberry pi.
		if (cmd == "pattern1" && val == 1){
			
		}

		loop_rate.sleep();
		ros::spinOnce();
	}

	return 0;
}


