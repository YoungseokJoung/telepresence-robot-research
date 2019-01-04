#include "ros/ros.h"
#include <sstream>
#include <visualization_msgs/Marker.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

#define _CRT_NONSTDC_NO_DEPRECATE

#include <assert.h>
#include <time.h>
#include <ctime>
#include "socket.h"
#include "gason.h"
#include "json_message.h"

unsigned short port = 2001;

using namespace std;

float left_elbow[3];
char left_elbow_char[20];

void messageCb(visualization_msgs::Marker marker){
        left_elbow[0] = marker.points[4].x;
        left_elbow[1] = marker.points[4].y;
        left_elbow[2] = marker.points[4].z;
}

int main(int argc, char **argv){

        ros::init(argc, argv, "skeleton_sub_json_server");
        ros::NodeHandle nh;
        ros::Subscriber marker_sub = nh.subscribe<visualization_msgs::Marker>("skeleton_markers", 100, messageCb);
        ros::Rate loop_rate(10);

	string argv_str(argv[0]);
	string path = argv_str.substr(0, argv_str.find_last_of("/"));

	for (int i = 1; i < argc && argv[i][0] == '-'; i++){
		switch (argv[i][1]){
			case 'p':
			port = atoi(argv[i + 1]);
			i++;
			break;
			case 'd':
			set_daemon(path.c_str());
			break;
		}
	}

	tcp_server_t server(port);
	cout << "server listening on port " << port << endl;
        stringstream ss;

        while(ros::ok()){
		char *str_ip;
		socket_t socket = server.accept_client();

		// convert IP addresses from a dots-and-number string to a struct in_addr and back
		str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
		cout << prt_time() << "server accepted: " << str_ip << " <" << socket.m_socket_fd << "> " << endl;

		string str_request = read_response(socket);
		cout << prt_time() << "server received: " << str_request << endl;
		char* buf_request = strdup(str_request.c_str());

		//get dates
		char *endptr;
		JsonValue value;
		JsonAllocator allocator;
		int status = jsonParse(buf_request, &endptr, &value, allocator);
		if (status != JSON_OK){
			cout << "invalid JSON format for " << buf_request << endl;
			return -1;
		}

		int year;
		for (JsonNode *node = value.toNode(); node != nullptr; node = node->next){
			cout << node->key;
			year = (int)node->value.toNumber();
		}

		string json = "{\"next_year\":";
		json += to_string(year + 1);
		json += "}";
		if (write_request(socket, json.c_str()) < 0){}
		cout << prt_time() << "server sent: " << json.c_str() << endl;

		socket.close_socket();
		free(buf_request);


                ros::spinOnce();
	}

	server.close_socket();

	return 0;
}

/*                ss<<left_elbow[0];

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

*/
