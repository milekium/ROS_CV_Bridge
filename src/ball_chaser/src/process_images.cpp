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
  	sub_ = n_.subscribe("/camera/rgb/image_raw", 1, &SubscribeAndPublish::callback, this);

  }

  void callback(const sensor_msgs::Image img)
  {

		int white_pixel = 255;	
		int black_pixel = 0;	
		int mid_img = img.width/2;
		//ball chase
    for (int i = 0; i < img.height * img.step; i++) {
				if (img.data[i] == white_pixel) {						
						int c = i/img.step;
						int r = (i-(c*img.step))/3;
						if (r <= (mid_img-(mid_img*0.2))){
								srv_.request.angular_z = 0.15;
								srv_.request.linear_x = 0.05;
								srv_.request.linear_y = 0.01;		
						}else if (r >= (mid_img+(mid_img*0.2))){	
								srv_.request.angular_z = -0.15;	
								srv_.request.linear_x = 0.05;
								srv_.request.linear_y = -0.01;														
						}else{
								srv_.request.angular_z = srv_.request.angular_z;		
								srv_.request.linear_x = 0.1;
								srv_.request.linear_y = srv_.request.linear_y;
				    }		
				break;				
				}
		}

		//move on the path, don't fall.
		int check_pixel = 500 * img.step; 
		if( img.data[check_pixel] != black_pixel || img.data[check_pixel + (img.step* 1/3)] != black_pixel){ //check only two pixels
				srv_.request.linear_x = 0.02;		
				srv_.request.linear_y = -0.05;		
				srv_.request.angular_z = srv_.request.angular_z;	
		}else if ( img.data[check_pixel + img.step - 1] != black_pixel || img.data[check_pixel + (img.step * 2/3)] != black_pixel){				
				srv_.request.linear_x = 0.02;		
				srv_.request.linear_y =  0.05;		
				srv_.request.angular_z = srv_.request.angular_z;
		}else{
				srv_.request.linear_y = srv_.request.linear_y;				
				srv_.request.linear_x = srv_.request.linear_x;
				srv_.request.angular_z = srv_.request.angular_z;
		}
  
    
		// Call the safe_move service and pass the requested joint angles
    if (!client_.call(srv_))
        ROS_ERROR("Failed to call service safe_move");
  }

private:
  ros::NodeHandle n_; 
  ros::Subscriber sub_;
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
