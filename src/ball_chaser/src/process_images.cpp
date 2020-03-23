#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

class SubscribeAndPublish
{
public:
  SubscribeAndPublish()
  {
    // Define a client service capable of requesting services from command_robot
  	client_ = n_.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

  	// Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
  	sub_ = n_.subscribe("/camera/rgb/image_raw", 3, &SubscribeAndPublish::callback, this);

  }
  void callback(const sensor_msgs::Image img)
  {	
		//first reset velocity commands from previous loops.
		srv_.request.linear_x = 0.0;
		srv_.request.linear_y = 0.0;
		srv_.request.angular_z = 0.0;
		//define variables for pixel colors
		int white_pixel = 255;	
		int black_pixel = 0;	
		//ball chaser by locating the white ball in the camera screen.
    for (int n = 0; n < img.height * img.width; n++) {
				//transform the pixel number to the  pixel bit number.
				int i = n*3;
				//check if the pixel rbg is white.
				if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {						
						//Get the column number where the white ball was found. 
						//we use int type to get number of rows as integer value, not float.
						int row = n/img.width;
						//By substracting the integer number of the row to the pixel number, we get the column where the 
						//white ball was first found.
						int column = n-(row*img.width);
						//condition for white ball in the left area of the screen; left side of screen is measured from
						// screen middle column mid_img minus a value of 1/8 of the screen, this makes left side size of 
						// 1/3 of screen width.  
						if (column <= (img.width/4)){
								//set robot command to turn left (angular_z) in the direcction of the white ball.
								srv_.request.angular_z = 0.5;
								srv_.request.linear_y = -0.001;	
						//condition for white ball in the right area of the camera screen; 	
						}else if (column >= (img.width*3/4)){	
								//set robot command to turn right (angular_z) in the direcction of the white ball.
								srv_.request.angular_z = -0.5;	
								srv_.request.linear_y = 0.001;		
						//condition for white ball in the middle area of the camera screen; 													
						}else{
								//set robot command to stop turn velocity (angular_z).
								srv_.request.angular_z = 0.0;		
								srv_.request.linear_x = 0.15;								
				    }	
						break;									
				}
		}

		//move on the path without falling.		
		//check condition for robot close to the path edge, if so, move the robot away from the edge.
		//checking only one row of pixels, row number int check_pixel; if there is no path color in the
		//row, this means robot is getting close to the path edge.
		int check_row = img.height*12/16;
		//middle pixel of the image row to be check.
		int check_pixel = check_row * img.step; 
		for(int m = 0; m < img.width*(img.height - check_row); m++){
			//check from middle row pixel to both sides pixel of the camera image. this way we can approximate 
			//the desviation or angle of the robot with the path.
			if( img.data[check_pixel + m] != black_pixel){
				//get the side of the image detecting not path color
				int row = m/img.width;
				int column = m -(row*img.width);
				//found path edge too close to robot on the right side of the image, make the robot move away.
				if ( column <= img.width/2){	
					//found robot approaching edge in its left side, move away by giving the robot some planar X and Y velocity
					// plus angular z rotacion. 
					srv_.request.linear_x = 0.0;		
					srv_.request.linear_y = -0.1;		
					srv_.request.angular_z = 0.05;				
				}
				else{
					//found robot approaching edge in its left side, move away by giving the robot some planar X and Y velocity
					// plus angular z rotacion.
					srv_.request.linear_x = 0.0;		
					srv_.request.linear_y =  0.1;		
					srv_.request.angular_z = -0.05;
				}		
				break;
			}
		}

		//ROS_INFO_STREAM(srv_.request);
    
		// Call the safe_move service and pass the requested joint angles
    if (!client_.call(srv_))
        ROS_ERROR("Failed to call service safe_move");
  }

private:
  ros::NodeHandle n_; 
  ros::Subscriber sub_;
	ros::Subscriber odom_;
	ros::ServiceClient client_;
	ball_chaser::DriveToTarget srv_;
};

int main(int argc, char **argv)
{
  //Initiate ROS
  ros::init(argc, argv, "subscribe_and_publish");

  //Create an object of class SubscribeAndPublish that will take care of everything
  SubscribeAndPublish SAPObject;

  // Handle ROS communication events
  ros::spin();

  return 0;
}
