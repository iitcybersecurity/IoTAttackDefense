

cplusplus {{
#include <TransportAddress.h>
#include <OverlayKey.h>
#include <MyApplication.h>

}}


// import the IP Address class
class noncobject TransportAddress;
class noncobject OverlayKey;
class noncobject Flow;
class noncobject FlowContainer;


// first, we declare an enum to tell the message types apart:
enum MessageType 
{
    //MYMSG_FWD = 1;           // outgoing message
    //MYMSG_PONG = 2;           // returning message
    MYMSG_GET_OPENSHS_DATA = 3;
    MYMSG_GET_OPENSHS_RESPONSE = 4;
    MYMSG_PUT_OPENSHS_DATA = 5;
    MYMSG_PUT_CLASSIFICATION_DATA= 6;
    MYMSG_GET_CLASSIFICATION_DATA = 7;
    MYMSG_PUT_TEMPERATURE = 8;
    MYMSG_PUT_MOTION = 9;
    MYMSG_GET_TEMPERATURE = 10;
    MYMSG_GET_MOTION = 11;
    MYMSG_GET_TEMPERATURE_RESPONSE = 12;
    MYMSG_GET_MOTION_RESPONSE = 13;
    MYMSG_GET_CLASSIFICATION_DATA_RESPONSE = 14;
    MYMSG_PUT_REPUTATION = 15;
    MYMSG_GET_REPUTATION = 16;
    MYMSG_GET_REPUTATION_RESPONSE = 17;
    MYMSG_PUT_VOTE = 18;
    MYMSG_GET_VOTE = 19;
    MYMSG_GET_VOTE_RESPONSE = 20;
}


// now we declare the message
packet MyMessage 
{
    int type enum(MessageType);      // message type
    TransportAddress senderAddress;  // address of the node that sent the message
    OverlayKey key;
    int requesterNodeId;
    int targetNodeId;
    int ownerNodeId;
    
	string detectedValue;
	string temperatureValue;
	string motionValue;
	string classificationData[10];
	double reputationValue[2];
	double voteValue;
	bool isInvalidData;
	int TimeSlot;
	
//	FlowContainer flowContainer;
//	Flow flow;			//It contains the flow to send 
	
}








