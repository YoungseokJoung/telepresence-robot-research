cmake_minimum_required(VERSION 2.8.3)
project(skeleton_markers)

find_package(catkin REQUIRED COMPONENTS
	roscpp
	rospy
	tf
	geometry_msgs
	std_msgs
	visualization_msgs
	message_generation
)
	
# Find Orocos KDL
find_package(orocos_kdl REQUIRED)

# Find OpenNI
find_package(PkgConfig)
pkg_check_modules(OpenNI REQUIRED libopenni)

# Find Glut
find_package(GLUT REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED) 

link_directories(
  ${catkin_LIBRARY_DIRS}
  ${orocos_kdl_LIBRARY_DIRS}
  ${OpenNI_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${OPENGL_INCLUDE_DIR}
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenNI_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${orocos_kdl_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
)

add_message_files (
  FILES
  Skeleton.msg
  ## interceptor.msg
)

generate_messages(
  	DEPENDENCIES geometry_msgs std_msgs visualization_msgs
)

catkin_package(
#    INCLUDE_DIRS include #modified
    DEPENDS 
    CATKIN_DEPENDS 
	rospy
	roscpp
	message_runtime
	openni_camera
	openni_tracker
	tf
	visualization_msgs
	std_msgs
	geometry_msgs
)

add_executable(skeleton_tracker
  src/skeleton_tracker.cpp
  src/KinectController.cpp
  src/KinectDisplay.cpp)

target_link_libraries(skeleton_tracker
  ${catkin_LIBRARIES}
  ${OpenNI_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}  
  ${orocos_kdl_LIBRARIES})

set(SOURCES ${SOURCES} src/socket.h)
set(SOURCES ${SOURCES} src/socket.cpp)
set(SOURCES ${SOURCES} src/gason.h)
set(SOURCES ${SOURCES} src/gason.cpp)
set(SOURCES ${SOURCES} src/json_message.h)
set(SOURCES ${SOURCES} src/json_message.cpp)

# subscribing tf from openni_tracker
add_executable(${PROJECT_NAME}_subscribe src/tracker_receiver.cpp)
target_link_libraries(${PROJECT_NAME}_subscribe ${catkin_LIBRARIES})

# subscribing Marker from Markers_from_tf.py & transfer it as a string cmd
add_executable(skeleton_sub_server src/skeleton_sub_server.cpp)
target_link_libraries(skeleton_sub_server ${catkin_LIBRARIES})

add_executable(skeleton_sub_client src/skeleton_sub_client.cpp)
target_link_libraries(skeleton_sub_client ${catkin_LIBRARIES})


# subscribing Marker from Markers_from_tf.py & transfer it as a JSON cmd
add_executable(skeleton_sub_json_server src/skeleton_sub_json_server.cpp ${SOURCES})
target_link_libraries(skeleton_sub_json_server ${catkin_LIBRARIES})

add_executable(skeleton_sub_json_client src/skeleton_sub_json_client.cpp ${SOURCES})
target_link_libraries(skeleton_sub_json_client ${catkin_LIBRARIES})


#install(TARGETS skeleton_tracker RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

message("DEBUG variable catkin_INCLUDE_DIRS: ${catkin_INCLUDE_DIRS}")
message("DEBUG variable OpenNI_INCLUDE_DIRS: ${OpenNI_INCLUDE_DIRS}")
message("DEBUG variable GLUT_INCLUDE_DIRS: ${GLUT_INCLUDE_DIRS}")
message("DEBUG variable orocos_kdl_INCLUDE_DIRS: ${orocos_kdl_INCLUDE_DIRS}")

