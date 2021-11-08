#include <omnetpp.h>

#include "GlobalStatistics.h"

Define_Module(GlobalStatistics);

using namespace std;

const double GlobalStatistics::MIN_MEASURED = 0.1;

void GlobalStatistics::initialize()
{
      //globalNodesHandler = dynamic_cast<GlobalNodesHandler*>(simulation.getModuleByPath("globalObserver.globalFunctions[0].function"));

    numDevices = par("numDevices");
    //numSimNodes = par("targetOverlayTerminalNum");
    classificationSlotTime = par("classificationSlotTime");
    numMaliciousNodes = par("numMaliciousNodes");
    scenario = par("scenario");
    //statistics
       //isSensorDataArrayInvalid = 0;
       numSensorDataConsistencies = 0;
       numSensorDataInconsistencies = 0;
      // numInvalidSensorDataArrays = 0;
       invalidTemperatureDataGETNumber = 0;
       invalidSensorDataGETNumber = 0;
       invalidMotionDataGETNumber = 0;
       successfulSensorDataGETNumber = 0;
       successfulTemperatureDataGETNumber = 0;
       successfulMotionDataGETNumber = 0;
       invalidSensorDataPercentageMean.counter=0;
       invalidSensorDataPercentageMean.accumulator=0;

       cout<<"GLOBALSTATISTICS1 "<<endl; //Debug

    maximumClassificationCycles = par("maximumClassificationCycles");
    averageMaliciousTrustCollector=new double[maximumClassificationCycles];
    averageMaliciousTrustAccumulator=0;
    averageMaliciousTrustCounter=0;
    averageNormalTrustCollector=new double[maximumClassificationCycles];
    averageNormalTrustAccumulator=0;
    averageNormalTrustCounter=0;
    slotNumber = 0;

    sentKBRTestAppMessages = 0;
    deliveredKBRTestAppMessages = 0;

    measuring = measureNetwInitPhase = par("measureNetwInitPhase");
    measureStartTime = 0;

    currentDeliveryVector.setName("Current Delivery Ratio");
    cout<<"GLOBALSTATISTICS2 "<<endl; //Debug


    WATCH(measuring);
    WATCH(measureStartTime);
    WATCH(currentDeliveryVector);

    controlledSensorData=new bool*[numSimNodes]();
    controlledTemperatureData=new bool*[numSimNodes]();
    controlledMotionData=new bool*[numSimNodes]();
    controlledClassificationData = new bool*[numSimNodes]();
    controlledVoteData = new bool*[numSimNodes]();
    for(int i=0;i<numSimNodes;i++){
        controlledSensorData[i]= new bool[numSimNodes];
        controlledTemperatureData[i]= new bool[numSimNodes];
        controlledMotionData[i]= new bool[numSimNodes];
        controlledClassificationData[i]= new bool[numSimNodes];
        controlledVoteData[i] = new bool[numSimNodes];
        for(int j=0;j<numSimNodes;j++){
            controlledSensorData[i][j]=false;
            controlledTemperatureData[i][j]=false;
            controlledMotionData[i][j]=false;
            controlledClassificationData[i][j]=false;
            controlledVoteData[i][j] = false;
        }
    }
    cout<<"GLOBALSTATISTICS3 "<<endl; //Debug

    sensorDataArrayCollector= new char*[numDevices]();
    sensorDataArrayCounter = 0;
    voteMatrix= new int*[numDevices];
    for(int i=0;i<numDevices;i++){
        voteMatrix[i]=new int[numDevices];
        for(int j=0;j<numDevices;j++)
            voteMatrix[i][j]=NORMAL;
    }

    if(scenario == 31 || scenario == 32){
        //The voting mechanism must be scheduled for each slot time.
        InfoMessage* votes_count_timer = new InfoMessage("votes_count_timer");
        scheduleAt(simTime() + classificationSlotTime, votes_count_timer);
    }
    //In IDS-based attacks we evaluate the mean trust of malicious and non-malicious ndoes
    if(scenario == 41 || scenario == 5 || scenario == 6){
        InfoMessage* average_trust_evaluation_timer = new InfoMessage("average_trust_evaluation_timer");
        scheduleAt(simTime() + classificationSlotTime, average_trust_evaluation_timer);
    }

    cout<<"GLOBALSTATISTICS4 "<<endl; //Debug

}


void GlobalStatistics::addControlledDataPair(int maliciousNode,int victimNode, string dataType){

    if(dataType == "openshs")
            controlledSensorData[maliciousNode][victimNode]=true;
    if(dataType == "temperature")
            controlledTemperatureData[maliciousNode][victimNode]=true;
    if(dataType == "motion")
            controlledMotionData[maliciousNode][victimNode]=true;
    if(dataType == "classification_data")
        controlledClassificationData[maliciousNode][victimNode]=true;
    if(dataType == "classification_result")
            controlledVoteData[maliciousNode][victimNode]=true;


}

void GlobalStatistics::startMeasuring()
{
    if (!measuring) {
        measuring = true;
        measureStartTime = simTime();
    }
}
//the function is called to assess whether an inconsistency between the sensor data collected by each non-malicious node is present
void GlobalStatistics::compareSensorDataArrays(char* sensorDataArray){

    sensorDataArrayCollector[sensorDataArrayCounter] = sensorDataArray;
    sensorDataArrayCounter++;
    int invalidDataCounter=0;
    for(int i=0;i<numDevices;i++){
        if(sensorDataArray[i]=='3')
            invalidDataCounter++;
    }
    invalidSensorDataPercentageMean.counter++;
    invalidSensorDataPercentageMean.accumulator+=(double)invalidDataCounter/numDevices;

    if(sensorDataArrayCounter==numDevices-numMaliciousNodes){ //usare numero di nodi non malicious al posto di numSimNodes
        sensorDataArrayCounter=0;

        cout<<endl;
        for(int i=1; i<numDevices-numMaliciousNodes;i++){
            for(int j=0;j<numDevices;j++){  //Qua invece lasciare numSimNodes
                if(sensorDataArrayCollector[i-1][j]!=sensorDataArrayCollector[i][j]){
                   numSensorDataInconsistencies++;
                   cout<<"GLOBALSTATISTICS: numSensorDataConsistencies-> "<<numSensorDataConsistencies<<" numSensorDataInconsistencies-> "<<numSensorDataInconsistencies<<endl;
                   return;
                }
            }
        }
        numSensorDataConsistencies++;
        //Debug
        //cout<<"GLOBALSTATISTICS: numSensorDataConsistencies-> "<<numSensorDataConsistencies<<" numSensorDataInconsistencies-> "<<numSensorDataInconsistencies<<" numInvalidSensorDataArrays-> "<<numInvalidSensorDataArrays<<endl;
    }
}

void GlobalStatistics::notifyOrchestrator(int i,int j, int vote){
    //If node i votes again for j as a normal node the message is ignored.
    int anomalous=ANOMALOUS;
    int normal=NORMAL;
    int not_available=NOT_AVAILABLE;
    if(vote == normal)
        voteMatrix[i][j] = NORMAL;
    if(vote == anomalous)
            voteMatrix[i][j] = ANOMALOUS;
    if(vote == not_available)
            voteMatrix[i][j] = NOT_AVAILABLE;

}
void GlobalStatistics::registerTrust(int nodeId,int trust, bool isMalicious){
    if(isMalicious){
        averageMaliciousTrustAccumulator += trust;
        averageMaliciousTrustCounter++;
    }
    else{
        averageNormalTrustAccumulator += trust;
        averageNormalTrustCounter++;
    }
}


void GlobalStatistics::handleMessage(cMessage* msg)
{

    InfoMessage * myMsg = dynamic_cast < InfoMessage * > (msg);

    if (myMsg -> isName("average_trust_evaluation_timer")) {

        averageMaliciousTrustCollector[slotNumber] = averageMaliciousTrustAccumulator/averageMaliciousTrustCounter;
        averageNormalTrustCollector[slotNumber] = averageNormalTrustAccumulator/averageNormalTrustCounter;
        averageMaliciousTrustAccumulator=0;
        averageMaliciousTrustCounter=0;
        averageNormalTrustAccumulator=0;
        averageNormalTrustCounter =0;

        slotNumber++;
        if(slotNumber == maximumClassificationCycles)
            endSimulation();
        scheduleAt(simTime() + classificationSlotTime, myMsg);
        return;
    }
    if (myMsg -> isName("votes_count_timer")) {
           int counter_normal=0;
           int counter_anomalous=0;
           int anomalous=ANOMALOUS;
           int normal=NORMAL;
           int not_available=NOT_AVAILABLE;
           for(int l=0;l<numDevices;l++){
               for(int i=0;i<numDevices;i++){
                   if(i == l)
                       continue;
                   if(voteMatrix[i][l] == anomalous)
                       counter_anomalous++;
                   if(voteMatrix[i][l] == normal)
                       counter_normal++;
                   voteMatrix[i][l]=not_available;
               }
               if(counter_anomalous>counter_normal){
                   cout<<"DELETING NODE "<<l<<endl;
                   delete msg; // unknown!
                   endSimulation();
               }
               cout<<"node"<<l<<" ->counter_anomalous="<<counter_anomalous<<" counter_normal="<<counter_normal<<endl;
               counter_anomalous=0;
               counter_normal=0;
           }
           cout<<endl;
           scheduleAt(simTime() + classificationSlotTime, myMsg);
           return;
       }
    error("GlobalStatistics::handleMessage(): Unknown message type!");
}

void GlobalStatistics::finish()
{
    // Here, the FinisherModule is created which will get destroyed at last.
    // This way, all other modules have sent their statistical data to the
    // GobalStatisticModule before GlobalStatistics::finalizeStatistics()
    // is called by FinisherModule::finish()
    cModuleType* moduleType = cModuleType::get("oversim.common.FinisherModule");
    moduleType->create("finisherModule", getParentModule()->getParentModule());
}


void GlobalStatistics::finalizeStatistics()
{
    int controlledTemperatureDataCounter=0;
        for(int i=0;i<numSimNodes;i++)
            for(int j=0;j<numSimNodes;j++)
                if(controlledTemperatureData[i][j]==true){
                    stringstream ssi;
                    ssi << i;
                    string str_i = ssi.str();
                    stringstream ssj;
                    ssj << j;
                    string str_j = ssj.str();
                    string s="node "+str_i+" controls temperature data of node"+str_j;
                    recordScalar(s.c_str(),true);
                    controlledTemperatureDataCounter++;
    }
    int controlledMotionDataCounter=0;
        for(int i=0;i<numSimNodes;i++)
            for(int j=0;j<numSimNodes;j++)
                if(controlledMotionData[i][j]==true){
                    stringstream ssi;
                    ssi << i;
                    string str_i = ssi.str();
                    stringstream ssj;
                    ssj << j;
                    string str_j = ssj.str();
                    string s="node "+str_i+" controls motion data of node"+str_j;
                    recordScalar(s.c_str(),true);
                    controlledMotionDataCounter++;
    }
    int controlledClassificationDataCounter=0;
            for(int i=0;i<numSimNodes;i++)
                for(int j=0;j<numSimNodes;j++)
                    if(controlledClassificationData[i][j]==true){
                        stringstream ssi;
                        ssi << i;
                        string str_i = ssi.str();
                        stringstream ssj;
                        ssj << j;
                        string str_j = ssj.str();
                        string s="node "+str_i+" controls classification data of node"+str_j;
                        recordScalar(s.c_str(),true);
                        controlledClassificationDataCounter++;
        }
    int controlledVoteDataCounter=0;
                for(int i=0;i<numSimNodes;i++)
                    for(int j=0;j<numSimNodes;j++)
                        if(controlledVoteData[i][j]==true){
                            stringstream ssi;
                            ssi << i;
                            string str_i = ssi.str();
                            stringstream ssj;
                            ssj << j;
                            string str_j = ssj.str();
                            string s="node "+str_i+" controls vote of node"+str_j;
                            recordScalar(s.c_str(),true);
                            controlledVoteDataCounter++;
            }

    int controlledSensorDataCounter=0;
    for(int i=0;i<numSimNodes;i++)
        for(int j=0;j<numSimNodes;j++)
            if(controlledSensorData[i][j]==true){
                stringstream ssi;
                ssi << i;
                string str_i = ssi.str();
                stringstream ssj;
                ssj << j;
                string str_j = ssj.str();
                string s="node "+str_i+" controls sensor data of node"+str_j;
                recordScalar(s.c_str(),true);
                controlledSensorDataCounter++;
            }

    recordScalar("invalidSensorDataGETNumber", invalidSensorDataGETNumber);
    recordScalar("invalidTemperatureDataGETNumber", invalidTemperatureDataGETNumber);
    recordScalar("invalidMotionDataGETNumber", invalidMotionDataGETNumber);
    recordScalar("successfulSensorDataGETNumber", successfulSensorDataGETNumber);
    recordScalar("successfulTemperatureDataGETNumber", successfulTemperatureDataGETNumber);
    recordScalar("successfulMotionDataGETNumber", successfulMotionDataGETNumber);
   // recordScalar("numDeliveredDataByMalicious", numDeliveredDataByMalicious);
    //recordScalar("numDeliveredDataByNonMalicious", numDeliveredDataByNonMalicious);

/*
    recordScalar("percentage of controlled sensor data", numMaliciousNodes + controlledSensorDataCounter);
    recordScalar("percentage of fixed portion of controlled sensor data", numMaliciousNodes );
    recordScalar("percentage of variable portion of controlled sensor data", controlledSensorDataCounter);
    recordScalar("numSensorDataConsistencies", numSensorDataConsistencies);
    recordScalar("numSensorDataInconsistencies", numSensorDataInconsistencies);
*/
    recordScalar("invalidSensorDataPercentageMean", (double)invalidSensorDataPercentageMean.accumulator/invalidSensorDataPercentageMean.counter);
    recordScalar("GlobalStatistics: Simulation Time", simTime());

    //Output of the mean trust for normal and malicious nodes
    for(int i=0;i<slotNumber-1;i++){
        averageMaliciousTrustVector.record(averageMaliciousTrustCollector[i]);
    }
    for(int i=0;i<slotNumber-1;i++){
        averageNormalTrustVector.record(averageNormalTrustCollector[i]);
    }

    bool outputMinMax = par("outputMinMax");
    bool outputStdDev = par("outputStdDev");

    // record stats from other modules
    for (map<std::string, cStdDev*>::iterator iter = stdDevMap.begin();
            iter != stdDevMap.end(); iter++) {

        const std::string& n = iter->first;
        const cStatistic& stat = *(iter->second);

        recordScalar((n + ".mean").c_str(), stat.getMean());

        if (outputStdDev)
            recordScalar((n + ".stddev").c_str(), stat.getStddev());

        if (outputMinMax) {
            recordScalar((n + ".min").c_str(), stat.getMin());
            recordScalar((n + ".max").c_str(), stat.getMax());
        }
    }

    for (map<std::string, cHistogram*>::iterator iter = histogramMap.begin();
            iter != histogramMap.end(); iter++) {
        const std::string& n = iter->first;
        recordStatistic(n.c_str(), iter->second);
    }

    for (map<std::string, OutVector*>::iterator iter = outVectorMap.begin();
    iter != outVectorMap.end(); iter++) {
        const OutVector& ov = *(iter->second);
        double mean = ov.count > 0 ? ov.value / ov.count : 0;
        recordScalar(("Vector: " + iter->first + ".mean").c_str(), mean);
    }
}

void GlobalStatistics::addStdDev(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, cStdDev*>::iterator sdPos = stdDevMap.find(name);
    cStdDev* sd = NULL;

    if (sdPos == stdDevMap.end()) {
        Enter_Method_Silent();
        sd = new cStdDev(name.c_str());
        stdDevMap.insert(pair<std::string, cStdDev*>(name, sd));
    } else {
        sd = sdPos->second;
    }

    sd->collect(value);
}

void GlobalStatistics::recordHistogram(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, cHistogram*>::iterator hPos = histogramMap.find(name);
    cHistogram* h = NULL;

    if (hPos == histogramMap.end()) {
        Enter_Method_Silent();
        h = new cHistogram(name.c_str());
        histogramMap.insert(pair<std::string, cHistogram*>(name, h));
    } else {
        h = hPos->second;
    }

    h->collect(value);
}

void GlobalStatistics::recordOutVector(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, OutVector*>::iterator ovPos =
        outVectorMap.find(name);
    OutVector* ov = NULL;

    if (ovPos == outVectorMap.end()) {
        Enter_Method_Silent();
        ov = new OutVector(name);
        outVectorMap.insert(pair<std::string, OutVector*>(name, ov));
    } else {
        ov = ovPos->second;
    }

    ov->vector.record(value);
    ov->value += value;
    ov->count++;
}

simtime_t GlobalStatistics::calcMeasuredLifetime(simtime_t creationTime)
{
    return simTime() - ((creationTime > measureStartTime)
            ? creationTime : measureStartTime);
}


GlobalStatistics::~GlobalStatistics()
{
    // deallocate vectors
    for (map<std::string, cStdDev*>::iterator it = stdDevMap.begin();
            it != stdDevMap.end(); it++) {
        delete it->second;
    }
    stdDevMap.clear();

    for (map<std::string, OutVector*>::iterator it = outVectorMap.begin();
            it != outVectorMap.end(); it++) {
        delete it->second;
    }
    outVectorMap.clear();

    for (map<std::string, cHistogram*>::iterator iter = histogramMap.begin();
            iter != histogramMap.end(); iter++) {
        delete iter->second;
    }
    histogramMap.clear();
}

