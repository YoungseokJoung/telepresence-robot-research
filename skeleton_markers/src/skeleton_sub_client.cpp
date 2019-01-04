#include "ros/ros.h"
#include <sstream>
#include <visualization_msgs/Marker.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <float.h>

#define PORT 3170

using namespace std;

float left_elbow[3];
char left_elbow_char[20];

void messageCb(visualization_msgs::Marker marker){

	left_elbow[0] = marker.points[4].x;
        left_elbow[1] = marker.points[4].y;
        left_elbow[2] = marker.points[4].z;

}

int main(int argc, char **argv){
	ros::init(argc, argv, "skeleton_sub_client");
	ros::NodeHandle nh;
    ros::Subscriber marker_sub = nh.subscribe<visualization_msgs::Marker>("skeleton_markers", 100, messageCb);
//	ros::Subscriber center_auto_cmd = nh.subscribe("cmd_from_hive" 100, );
	ros::Rate loop_rate(10);

	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;


//	valiables for base light
	int light_value = 0;
	char light_val_char[20];
	std::string color_value;

//	cmd buffer
	char buf_json[257] = "";

	char buffer[1024];
	float left_elbow_x;

	// socket connection -> make function for this later
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	printf("\n Socket creation error \n");
	return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "192.168.1.127", &serv_addr.sin_addr)<=0) 
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
	printf("\nConnection Failed \n");
	return -1;
	}

	stringstream torque;

	for(int i = 5; i < 8; i++){
//		std::string tmp = std::to_string(i);
	    torque << std::string("torque ") + std::to_string(i) + " on\n"; //c++11
        strncpy(buf_json, torque.str().c_str(), sizeof(buf_json));
		if(send(sock, buf_json, strlen(buf_json), 0) < 0)
                                std::cout<<"send failed"<<endl;
                else{
                                printf("cmd = %s", buf_json);
                                usleep(100000);
                }
                torque.str("");
                torque.clear();

	}

	stringstream ss;
	stringstream light_val;
	stringstream tmp_str;

    for (int i = 0; i < sizeof(buf_json); i++) {
            buf_json[i] = '\0';
    }

	while(ros::ok()){

        //sending cordinate
        ss<<left_elbow[0];
        strncpy(left_elbow_char, ss.str().c_str(), 20);

		for(int i = 0; i < 3; i++){
			light_value = rand() % 255 + 0;
			light_val<<light_value;
			strncpy(light_val_char, light_val.str().c_str(), sizeof(light_val_char));
			color_value = color_value + " " + light_val_char;
			light_val.str("");
                        light_val.clear();
		}

		//make cmd
        tmp_str << std::string("light_color 20") + color_value + "\n";
        strncpy(buf_json, tmp_str.str().c_str(), sizeof(buf_json));
        color_value.clear();

	    if(recv(sock, buffer, sizeof(buffer)/sizeof(*buffer), 0) < 0)
			std::cout<<"recv failed";
		else{
		    printf("from server : %s", buffer);
        	if(send(sock, buf_json, strlen(buf_json), 0) < 0)
				std::cout<<"send failed"<<endl;
			else{
			    printf("cmd = %s", buf_json);
				usleep(100000);
			}
		}

		
		for (int i = 0; i < sizeof(buf_json); i++) {
	        buf_json[i] = '\0';
		}
		for (int i = 0; i < sizeof(buffer); i++) {
	        buffer[i] = '\0';
		}
	    ss.str("");
	    ss.clear();
		light_val.str("");
		light_val.clear();
		tmp_str.str("");
		tmp_str.clear();

		loop_rate.sleep();
		

        ros::spinOnce();
	}

        return 0;
}
