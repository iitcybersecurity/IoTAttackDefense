
package oversim.common;

//
// Module to record global statistics
//
simple GlobalStatistics
{
    parameters:
        bool outputMinMax;  // enable output of minimum and maximum for scalars
        bool outputStdDev;  // enable output of standard deviation for scalars
        
        //cOutVector averageMaliciousTrustVector;
        //cOutVector averageNormalTrustVector;
        
        bool measureNetwInitPhase;    // fetch statistics in init phase?
        int numSimNodes;
        double globalStatTimerInterval @unit(s);    // interval length of periodic statistic timer
        int numTemperatureDevices;
        int numMotionDevices;
       // double maliciousNodesPercentage;
        int numMaliciousNodes;
        int targetNumMaliciousNodes;
        int numVictimNodes;
        int groupA_nodesNumber;
        int groupB_nodesNumber;
        int addMaliciousNodePeriod;
        int scenario;
        int classificationSlotTime;
        int maximumClassificationCycles;
        int numDevices;
}






