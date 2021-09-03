//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// @author Antonio Zea
//

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
    MYMSG_GET_TOPOLOGY1_DATA = 3;
    MYMSG_GET_TOPOLOGY1_RESPONSE = 4;
    MYMSG_PUT_TOPOLOGY1_DATA = 5;
    MYMSG_PUT_FLOW = 6;
    MYMSG_GET_FLOW = 7;
    MYMSG_PUT_TEMPERATURE = 8;
    MYMSG_PUT_MOTION = 9;
    MYMSG_GET_TEMPERATURE = 10;
    MYMSG_GET_MOTION = 11;
    MYMSG_GET_TEMPERATURE_RESPONSE = 12;
    MYMSG_GET_MOTION_RESPONSE = 13;
    MYMSG_GET_FLOW_RESPONSE = 14;
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
	string flowData[10];
	double reputationValue[2];
	double voteValue;
	bool isInvalidData;
	int TimeSlot;
	
//	FlowContainer flowContainer;
//	Flow flow;			//It contains the flow to send 
	
}








