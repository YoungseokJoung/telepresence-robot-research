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

#define PORT 8080

using namespace std;

float left_elbow[3];
char left_elbow_char[20];

void messageCb(visualization_msgs::Marker marker){

	left_elbow[0] = marker.points[4].x;
        left_elbow[1] = marker.points[4].y;
        left_elbow[2] = marker.points[4].z;

}

int main(int argc, char **argv){

        ros::init(argc, argv, "skeleton_sub");
        ros::NodeHandle nh;
        ros::Subscriber marker_sub = nh.subscribe<visualization_msgs::Marker>("skeleton_markers", 100, messageCb);
	ros::Rate loop_rate(10);

	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "response from Computer B (server)";


        stringstream ss;

	// Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
                perror("socket failed");
                exit(EXIT_FAILURE);
        }
        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
                perror("accept");
                exit(EXIT_FAILURE);
        }

	while(ros::ok()){

		ss<<left_elbow[0];

        	strncpy(left_elbow_char, ss.str().c_str(), 20);
	        valread = recv(new_socket, buffer, sizeof(buffer)/sizeof(*buffer), 0);

		if (valread!=0){
		        printf("from client : %s\n", buffer);
        		send(new_socket, left_elbow_char, strlen(left_elbow_char), 0 );
		        printf("left_elbow = %s sent\n",left_elbow_char);
		        buffer[0] = '\0';
		        ss.str("");
		        ss.clear();
			loop_rate.sleep();
		}

        	ros::spinOnce();
	}

        return 0;
}
