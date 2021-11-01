
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
        //int numSimNodes;
        int targetOverlayTerminalNum;
        double globalStatTimerInterval @unit(s);    // interval length of periodic statistic timer
        int numTemperatureDevices;
        int numMotionDevices;
       // double maliciousNodesPercentage;
        double numMaliciousNodes;
        double targetNumMaliciousNodes;
        double numVictimNodes;
        int addMaliciousNodePeriod;
        int scenario;
        int classificationSlotTime;
        int maximumClassificationCycles;
}
