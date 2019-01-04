#include "ros/ros.h"
#include "center_bridge/cmd_from_hive.h"
#include "center_bridge/cmd_to_raspberry.h"
#include <sstream>

#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>


#define PORT 1234

using namespace std;

int main(int argc, char **argv){

	//ros
	ros::init(argc, argv, "aws_center_bridge");
	ros::NodeHandle nh;
	ros::Publisher aws_center_bridge_pub = nh.advertise<center_bridge::cmd_from_hive>("cmd_from_hive",10);
	ros::Publisher center_rasp_bridge_pub = nh.advertise<center_bridge::cmd_to_raspberry>("cmd_to_raspberry",10);
	ros::Rate loop_rate(10);

	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	center_bridge::cmd_from_hive cmd_from_hive;
	center_bridge::cmd_to_raspberry cmd_to_raspberry;

	//cmd buffer for socket connection with hive
	char to_server[257] = "cmd received\n";
	char from_server[1024];
	char *pch;
	int val;
	string type;
	string tmp;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	printf("\n Socket creation error \n");
	return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, /*"127.0.0.1"*/"18.144.26.211", &serv_addr.sin_addr)<=0) 
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	//main loop receiving cmd from hive
	while(ros::ok()){

		//check server response for previous cmd
		if(recv(sock, from_server, sizeof(from_server)/sizeof(*from_server), 0) < 0)
			std::cout<<"recv failed";
		else{
		    printf("from server : %s", from_server);
		}

		//send response to server   
		if(send(sock, to_server, strlen(to_server), 0) < 0)
			std::cout<<"send failed"<<endl;
		else{
	    		printf("cmd = %s", to_server);
			usleep(1000);
		}
			
		// split cmd string (if cmd type is mode, save cmd and value)
		pch = strtok(from_server," ");

		type = std::string(pch);
		
		if(type == "mode" || type == "set"){
			pch = strtok(NULL," ");
			cmd_from_hive.cmd = pch; //std::string(pch); //tmp_cmd.str.c_str(); // cmd
			printf ("put %s in the message\n",pch);
				
			pch = strtok(NULL," ");
			val = atoi(pch); // on/off
			cmd_from_hive.val = val;
			printf ("put %d in the message\n", val);
			
			ROS_INFO("publishing command %s %d \n", cmd_from_hive.cmd.c_str(), cmd_from_hive.val);
			aws_center_bridge_pub.publish(cmd_from_hive);
		}
		
		if(type == "light"){

			pch = strtok(NULL," ");
			tmp = std::string(pch);

			if(tmp == "power"){
				cmd_to_raspberry.cmd = pch; //std::string(pch); //tmp_cmd.str.c_str(); // cmd
				printf ("put %s in the message\n",pch);

				pch = strtok(NULL, " ");
				val = atoi(pch); // on/off
				cmd_to_raspberry.val = val;
				printf ("put %d in the message\n", val);				

				ROS_INFO("publishing command %s %d \n", cmd_to_raspberry.cmd.c_str(), cmd_to_raspberry.val);
				center_rasp_bridge_pub.publish(cmd_to_raspberry);
			}else if(tmp == "color"){

			}
		}

		bzero(from_server, 258);
		bzero(to_server, 258);
		
		loop_rate.sleep();
		ros::spinOnce();
	}

  return 0;
}
