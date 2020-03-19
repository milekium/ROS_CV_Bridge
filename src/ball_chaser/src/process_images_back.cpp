#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float lin_y, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.linear_y = lin_y;
    srv.request.angular_z = ang_z;

    
    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service safe_move");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
		// TODO: Loop through each pixel in the image and check if there's a bright white one

		int white_pixel = 255;	
		int black_pixel = 0;	
		int mid_img = img.width/2;
		//ball chase
    for (int i = 0; i < img.height * img.step; i++) {
				if (img.data[i] == white_pixel) {						
						int c = i/img.step;
						int r = (i-(c*img.step))/3;
						if (r <= (mid_img-(mid_img*0.05))){
								drive_robot(0.02, -0.02, 0.15);
						}else if (r >= (mid_img+(mid_img*0.05))){								
								drive_robot(0.02, 0.02, -0.15);
						}else{
								drive_robot(0.15, 0, 0);			
				    }		
				break;				
				}
		}
		//move on the path, don't fall.
		int check_pixel = 500 * img.step; 
		if( img.data[check_pixel] != black_pixel ){
				drive_robot(0.0, -0.05, -0.3);
				ros::Duration(1).sleep();
		}else if ( img.data[check_pixel + img.step - 1] != black_pixel){		
				drive_robot(0.0, 0.05, 0.3);
				ros::Duration(1).sleep();
		}else{
				drive_robot(0.15, 0, 0);
		}
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_images");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 1, process_image_callback);
	
    // Handle ROS communication events
    ros::spin();

    return 0;
}
