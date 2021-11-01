
import oversim.common.BaseApp;
import oversim.common.ITier;

package oversim.applications.myapplication;

simple MyApplication extends BaseApp
{
    parameters:
        @class(MyApplication);
        int classificationSlotTime;
        //int numSimNodes;           
        int targetOverlayTerminalNum;
        int numReplicas;
        int scenario;
     //   bool scenario2attack;
   //      bool scenario5attack;
   //      bool scenario6attack;
  //       bool isDataIntegrityEnabled;
        int topology;
        int putTemperaturePeriod;
        int putMotionPeriod;
        double readTemperatureProbability;
        double readMotionProbability;
        double trustThreshold;
    	double reputationThreshold;
    	double beliefVar;
    	double disbeliefVar;
    	double wrongClassificationProbability;
    	int reputationStrikesLimit;
    	bool isSybilEnabled;
}

module MyApplicationModules like ITier
{
    gates:
        input udpIn;             // gate from the UDP layer
        output udpOut;           // gate to the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        output to_lowerTier;     // gate to the lower tier
        output to_upperTier;     // gate to the upper tier
        input trace_in;          // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer

    submodules:
        application: MyApplication;

    connections allowunconnected:
        from_lowerTier --> application.from_lowerTier;
        to_lowerTier <-- application.to_lowerTier;
        udpIn --> application.udpIn;
        udpOut <-- application.udpOut;
        tcpIn --> application.tcpIn;
        tcpOut <-- application.tcpOut;
        trace_in --> application.trace_in;
}
