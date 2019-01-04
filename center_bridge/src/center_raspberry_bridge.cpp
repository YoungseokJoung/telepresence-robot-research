// subscribe from center + client requesting to raspberry pi.

#include "center_bridge/cmd_to_raspberry.h"
#include "ros/ros.h"
#include <sstream>

//socket
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

#define PORT 2000

using namespace std;

//cmd from center
string cmd;
int val;

//subscriber
void messageCb(center_bridge::cmd_to_raspberry cmd_to_raspberry){
	ROS_INFO("message_recieved : %s %d", cmd_to_raspberry.cmd.c_str(), cmd_to_raspberry.val);
	cmd = cmd_to_raspberry.cmd.c_str();
	val = cmd_to_raspberry.val;
}

int main(int argc, char **argv){
	ros::init(argc, argv, "center_raspberry_bridge");
	ros::NodeHandle nh;
	ros::Subscriber center_sub = nh.subscribe("cmd_to_raspberry", 100, messageCb);
	ros::Rate loop_rate(10);

	//socket connection
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	bool connection = false;
	
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	//cmd buffer
	char cmd2pi[257] = "";

	//for light value
	stringstream light;

	//stringstream 
	stringstream tmp_str;
	stringstream tmp_cmd;

	while(ros::ok()){
		if(cmd == "power" && val == 0 && connection == true) {

			connection = false;
			//make light on signal cmd
			light << std::string("0\n");// + /*std::to_string(i) +*/" on\n"; //c++11
	        	strncpy(cmd2pi, light.str().c_str(), sizeof(cmd2pi));
	        
		        //send light value through cmd
			if(send(sock, cmd2pi, strlen(cmd2pi), 0) < 0)
				std::cout<<"send failed"<<endl;
		        else {
		        	printf("cmd = %s", cmd2pi);
				usleep(500000);
			}

			light.str("");
			light.clear();

			//clear buffer
			for (int i = 0; i < sizeof(cmd2pi); i++) {
		        	cmd2pi[i] = '\0';
			}

			//disconnect client from robot
			close(sock);
		}

		if(cmd == "power" && val == 1 && connection == false) {
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
				printf("\n Socket creation error \n");
				return -1;
			}

			// Convert IPv4 and IPv6 addresses from text to binary form
			if(inet_pton(AF_INET, /*"127.0.0.1""192.168.2.10"*/"192.168.0.37", &serv_addr.sin_addr)<=0) {
				printf("\nInvalid address/ Address not supported \n");
				return -1;
			}

			if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
				printf("\nConnection Failed \n");
				return -1;
			}

			for(int a = 0; a < 10; a ++){
				//make light on signal cmd
				light << std::string("1\n");// + /*std::to_string(i) +*/" on\n"; //c++11
	        		strncpy(cmd2pi, light.str().c_str(), sizeof(cmd2pi));
	        

			        //send light value through cmd
				if(send(sock, cmd2pi, strlen(cmd2pi), 0) < 0)
					std::cout<<"send failed"<<endl;
			        else {
			        	printf("cmd = %s", cmd2pi);
					usleep(500000);
				}

				light.str("");
				light.clear();

				//clear buffer
				for (int i = 0; i < sizeof(cmd2pi); i++) {
			        	cmd2pi[i] = '\0';
				}
			}

			tmp_str.str("");
			tmp_str.clear();
			tmp_cmd.str("");
			tmp_cmd.clear();

			connection = true;
		}

		loop_rate.sleep();
		ros::spinOnce();
	}

	return 0;
	
}
