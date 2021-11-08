
package oversim.common;


simple GlobalNodesHandler like IGlobalFunctions
{
    parameters:
        @display("t=GlobalNodesHandler");
	int numSimNodes;       
	int scenario;
	int application;
	int numTemperatureDevices;
    int numMotionDevices;
   int numMaliciousNodes;
  int targetNumMaliciousNodes;
    int numVictimNodes;
    int groupA_nodesNumber;
    int groupB_nodesNumber;
    int numSybils;
    int numDevices;
    int classificationSlotTime;
}
