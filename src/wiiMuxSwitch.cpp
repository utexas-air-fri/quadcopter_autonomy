#include "ros/ros.h"
#include "sensor_msgs/Joy.h"
#include "sensor_msgs/JoyFeedbackArray.h"
#include "sensor_msgs/JoyFeedback.h"
#include "topic_tools/MuxSelect.h"
#include "topic_tools/MuxList.h"
#include <string>

bool listenState;
int currentTopic;
ros::V_string topics;
ros::Publisher wiimote_state;

void displayNumber(int n) {
	sensor_msgs::JoyFeedbackArray joyFeedback;
	std::vector<sensor_msgs::JoyFeedback> leds;
	sensor_msgs::JoyFeedback f;

	for (int i = 0; i < 4; i ++) {
		f.type = 0;
		f.id = i;
		f.intensity = i == n ? 1 : 0;
		leds.push_back(f); 
	}

	joyFeedback.array = leds;
	wiimote_state.publish(joyFeedback);
}

void joyCallback(const sensor_msgs::Joy::ConstPtr& msg){

	bool bState = msg->buttons[3] == 1;

	if (bState != listenState) {

		if (bState) {
			
			listenState = bState;

			++currentTopic;
			if (currentTopic >= topics.size()) currentTopic = 0;

			topic_tools::MuxSelect m;
			m.request.topic = topics[currentTopic];
			ros::service::call("/mux/select", m);
			displayNumber(currentTopic);
			printf("Switching mux to topic #%d: %s\n", currentTopic, topics[currentTopic].c_str());

		} else listenState = false;	

	}
	
}

int main(int argc, char **argv){

	ros::init(argc, argv, "wiiMuxSwitch");
	ros::NodeHandle n;

	//ask mux service for the list of topics to toggle between
	topic_tools::MuxList l;
	ros::service::call("/mux/list", l);
	topics = l.response.topics;

	currentTopic = 0;
	listenState = false;
	
	wiimote_state = n.advertise<sensor_msgs::JoyFeedbackArray>("/joy/set_feedback", 1000);
	ros::Subscriber sub = n.subscribe("joy", 1000, joyCallback);   

	printf("Press B to cycle through topics:\n");
	for (int i = 0; i < topics.size(); ++i) printf("\tTopic #%d: %s\n", i, topics[i].c_str());

	ros::spin();

	return 0;

}
