#!/bin/bash

# Build the catkin_ws
cd $(pwd)/../.. ; catkin_make

# Launch the nodes
xterm -e " source ./devel/setup.bash; source env.sh; roslaunch my_robot world.launch" &
sleep 5
xterm -e " source ./devel/setup.bash; roslaunch ball_chaser ball_chaser.launch" &
sleep 5
xterm -e " source ./devel/setup.bash; source env.sh; rosrun rviz rviz -d $ENV_SRC_FILES/rvizConfig/config.rviz "

# Termiante the project
echo "Press Enter key to terminate the project"
read -s -n 1 key
if [[ $key = "" ]]; then
	killall -e xterm
	echo "Project terminated!"
fi
