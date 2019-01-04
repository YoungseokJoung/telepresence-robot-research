#include "ros/ros.h"
#include <sstream>
#include <visualization_msgs/Marker.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <assert.h>
#include "socket.h"
#include "json_message.h"

unsigned short port = 5555;

using namespace std;

float left_elbow[3] = {0};
char left_elbow_char[20];

void messageCb(visualization_msgs::Marker marker){

	left_elbow[0] = marker.points[4].x;
        left_elbow[1] = marker.points[4].y;
        left_elbow[2] = marker.points[4].z;

}

int main(int argc, char **argv){

        ros::init(argc, argv, "skeleton_sub_json_client");
        ros::NodeHandle nh;
        ros::Subscriber marker_sub = nh.subscribe<visualization_msgs::Marker>("skeleton_markers", 100, messageCb);
	ros::Rate loop_rate(10);

        stringstream ss;

	char server[255]; // server host name or IP

	strcpy(server, "192.168.1.127");
//	strcpy(server, "127.0.0.1");
	for (int i = 1; i < argc && argv[i][0] == '-'; i++){
		switch (argv[i][1]){
			case 's':
				strcpy(server, argv[i + 1]);
			        i++;
		        break;
			case 'p':
			        port = atoi(argv[i + 1]);
			        i++;
			break;
		}
  	}

	//make JSON
//	char buf_json[257] = "{ \"type\": \"OhmniAPI\", \"jsoncmd\" : { \"cmd\": \"move\", \"lspeed\": 10, \"rspeed\": 10, \"time\": 2 }}";
	char buf_json[257] = "{ \"type\": \"OhmniAPI\", \"jsoncmd\" : { \"cmd\": \"setLightColor\", \"h\": 0, \"s\": 200, \"v\": 120 }}";
//	char buf_json[257] = '{"type":"OhmniAPI", "jsoncmd":{"setLightColor", "h":255, "s":255, "v":255}}';

	tcp_client_t client(server, port);
	std::cout << "client connecting to: " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

	//create socket and open connection
	if (client.open() < 0){
		std::cout << "connection failed";
	}
        std::cout << "client connected to: " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

	//write request
	if (write_request(client, buf_json) < 0){}
	std::cout << "client sent: ";
	std::cout << buf_json << " " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

        //read response
        std::string str_response = read_response(client);
        std::cout << "client received: ";
        std::cout << str_response << " " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

/*	//read response
	std::string str_response = read_response(client);
	std::cout << "client received: ";
	std::cout << str_response << " " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;
*/
	for (int i = 0; i < sizeof(buf_json); i++) {
	        buf_json[i] = '\0';
	}

	stringstream tmp_str;

	while(ros::ok()){

		//sending cordinate
		ss<<left_elbow[0];
        	strncpy(left_elbow_char, ss.str().c_str(), 20);
/*
		tmp_str << std::string("{") + "\"left_elbow_x\":" + left_elbow_char + "}";
        	strncpy(buf_json, tmp_str.str().c_str(), 20);
*/

		//create socket and open connection
		if (client.open() < 0){}
		std::cout << "client connected to: " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

		//write request
		if (write_request(client, buf_json) < 0){}
		std::cout << "client sent: ";
		std::cout << buf_json << " " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

		//read response
		std::string str_response = read_response(client);
		std::cout << "client received: ";
		std::cout << str_response << " " << server << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

		//close connection
		client.close_socket();

		//for (int i = 0; i < sizeof(buf_json); i++) {
		//        buf_json[i] = '\0';
		//}

	        ss.str("");
	        ss.clear();
		tmp_str.str("");
		tmp_str.clear();

		loop_rate.sleep();
        	ros::spinOnce();

	}


		
  return 0;
}


