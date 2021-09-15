#include <omnetpp.h>

#include "GlobalStatistics.h"
#include "MyApplication.h"

Define_Module(GlobalStatistics);

using namespace std;

const double GlobalStatistics::MIN_MEASURED = 0.1;

void GlobalStatistics::initialize()
{

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
       numDeliveredDataByMalicious=0;
       numDeliveredDataByNonMalicious=0;

       numSimNodes = par("targetOverlayTerminalNum");
       sensorDataArrayCollector= new char*[numSimNodes]();
       sensorDataArrayCounter = 0;
       numTemperatureDevices=par("numTemperatureDevices");
       numMotionDevices=par("numMotionDevices");
       //maliciousNodesPercentage = par("maliciousNodesPercentage");
       numMaliciousNodes = par("numMaliciousNodes");
       targetNumMaliciousNodes = par("targetNumMaliciousNodes");
       numVictimNodes = par("numVictimNodes");


    controlledSensorData=new bool*[numSimNodes]();
    controlledTemperatureData=new bool*[numSimNodes]();
    controlledMotionData=new bool*[numSimNodes]();
    controlledFlowData = new bool*[numSimNodes]();
    controlledVoteData = new bool*[numSimNodes]();
    for(int i=0;i<numSimNodes;i++){
        controlledSensorData[i]= new bool[numSimNodes];
        controlledTemperatureData[i]= new bool[numSimNodes];
        controlledMotionData[i]= new bool[numSimNodes];
        controlledFlowData[i]= new bool[numSimNodes];
        controlledVoteData[i] = new bool[numSimNodes];
        for(int j=0;j<numSimNodes;j++){
            controlledSensorData[i][j]=false;
            controlledTemperatureData[i][j]=false;
            controlledMotionData[i][j]=false;
            controlledFlowData[i][j]=false;
            controlledVoteData[i][j] = false;
        }
    }


    maliciousNodes=new bool[numSimNodes];
    victimNodes = new bool[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        maliciousNodes[i]=false;
        victimNodes[i]=false;
    }
    temperatureDevices=new int[numTemperatureDevices];
    motionDevices=new int[numMotionDevices];

    voteMatrix= new int*[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        voteMatrix[i]=new int[numSimNodes];
        for(int j=0;j<numSimNodes;j++)
            voteMatrix[i][j]=NORMAL;
    }

    sentKBRTestAppMessages = 0;
    deliveredKBRTestAppMessages = 0;

    measuring = measureNetwInitPhase = par("measureNetwInitPhase");
    measureStartTime = 0;

    currentDeliveryVector.setName("Current Delivery Ratio");

    // start periodic globalStatTimer
    addMaliciousNodePeriod = par("addMaliciousNodePeriod");
    if (addMaliciousNodePeriod > 0) {
        addMaliciousNodeTimer = new cMessage("addMaliciousNode");
        scheduleAt(simTime() + 310, addMaliciousNodeTimer);  //DEBUG  we add the new malicious node after the first reputation_update
    }

    WATCH(measuring);
    WATCH(measureStartTime);
    WATCH(currentDeliveryVector);

    setMaliciousNodes(numMaliciousNodes);
    setVictimNodes(numVictimNodes);

    setTemperatureAndMotionDevices(numTemperatureDevices,numMotionDevices);

    cout<<"GlobalStatistics: victim nodes -> ";
    for(int i=0;i<numSimNodes;i++)
        if(victimNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: initial malicious nodes -> ";
    for(int i=0;i<numSimNodes;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: temperature nodes -> ";
    for(int i=0;i<numTemperatureDevices;i++)
        cout<<" "<<temperatureDevices[i];
    cout<<endl;
    cout<<"GlobalStatistics: motion nodes -> ";
        for(int i=0;i<numMotionDevices;i++)
            cout<<" "<<motionDevices[i];
    cout<<endl;

}

void GlobalStatistics::addMaliciousNode(){
    int array[numSimNodes];
    for(int i=0;i<numSimNodes-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numSimNodes-1);
    int i=0;
    while(isMalicious(array[i]))   //I choose a random node that is not already malicious.
        i++;
    maliciousNodes[array[i]]=true;
    cout<<"Node"<<array[i]<<" is now malicious!"<<endl; //debug
    cout<<"GlobalStatistics: actual victim nodes -> ";  //debug
    for(int i=0;i<numSimNodes;i++)
        if(victimNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: actual malicious nodes -> ";   //debug
    for(int i=0;i<numSimNodes;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
    removeVictimNode(array[i]);    //I remove that node from the victim nodes list.
}
void GlobalStatistics::removeVictimNode(int victimNodeId){
    victimNodes[victimNodeId] = false;
}

void GlobalStatistics::setVictimNodes(int numVictimNodes){
    int array[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numSimNodes);
    int j=0;
    int i=0;
    while(j<numVictimNodes){
        if(!isMalicious(array[i])){
            victimNodes[array[i]] = true;
            j++;
        }
        i++;
    }

}

bool GlobalStatistics::isVictim(int nodeId){
    return victimNodes[nodeId];
}


void GlobalStatistics::setMaliciousNodes(int numMaliciousNodes){

    int array[numSimNodes];
    for(int i=0;i<numSimNodes-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numSimNodes-1);
    for(int i=0;i<numMaliciousNodes;i++)
        maliciousNodes[array[i]]=true;
}


void GlobalStatistics::setTemperatureAndMotionDevices(int numTemperatureDevices, int numMotionDevices){

    for(int i=0;i<numTemperatureDevices;i++)
        temperatureDevices[i]=i;
    for(int j=0;j<numMotionDevices;j++)
        motionDevices[j]=numTemperatureDevices+j;
}

int GlobalStatistics::getNumTemperatureDevices(){
    return numTemperatureDevices;
}
int GlobalStatistics::getNumMotionDevices(){
    return numMotionDevices;
}

int* GlobalStatistics::getTemperatureDevices(){
    return temperatureDevices;
}

int* GlobalStatistics::getMotionDevices(){
    return motionDevices;
}

void GlobalStatistics::addControlledDataPair(int maliciousNode,int victimNode, string dataType){

    if(dataType == "openshs")
            controlledSensorData[maliciousNode][victimNode]=true;
    if(dataType == "temperature")
            controlledTemperatureData[maliciousNode][victimNode]=true;
    if(dataType == "motion")
            controlledMotionData[maliciousNode][victimNode]=true;
    if(dataType == "flow")
            controlledFlowData[maliciousNode][victimNode]=true;
    if(dataType == "vote")
            controlledVoteData[maliciousNode][victimNode]=true;


}
bool GlobalStatistics::isMalicious(int nodeId){
    return maliciousNodes[nodeId];
}
bool GlobalStatistics::isTemperatureDevice(int nodeId){
    for(int i=0;i<numTemperatureDevices;i++)
        if(temperatureDevices[i]==nodeId)
            return true;
    return false;
}
bool GlobalStatistics::isMotionDevice(int nodeId){
    for(int i=0;i<numMotionDevices;i++)
            if(motionDevices[i]==nodeId)
                return true;
        return false;
}
void GlobalStatistics::startMeasuring()
{
    if (!measuring) {
        measuring = true;
        measureStartTime = simTime();
    }
}

void GlobalStatistics::compareSensorDataArrays(char* sensorDataArray){

    sensorDataArrayCollector[sensorDataArrayCounter] = sensorDataArray;
    sensorDataArrayCounter++;
    int invalidDataCounter=0;
    for(int i=0;i<numSimNodes;i++){
        if(sensorDataArray[i]=='3')
            invalidDataCounter++;
    }
    invalidSensorDataPercentageMean.counter++;
    invalidSensorDataPercentageMean.accumulator+=(double)invalidDataCounter/numSimNodes;

    if(sensorDataArrayCounter==numSimNodes-numMaliciousNodes){ //usare numero di nodi non malicious al posto di numSimNodes
        sensorDataArrayCounter=0;

        cout<<endl;
        for(int i=1; i<numSimNodes-numMaliciousNodes;i++){
            for(int j=0;j<numSimNodes;j++){  //Qua invece lasciare numSimNodes
                if(sensorDataArrayCollector[i-1][j]!=sensorDataArrayCollector[i][j]){
                   numSensorDataInconsistencies++;
                   cout<<"GLOBALSTATISTICS: numSensorDataConsistencies-> "<<numSensorDataConsistencies<<" numSensorDataInconsistencies-> "<<numSensorDataInconsistencies<<endl;
                   return;
                }
            }
        }
        numSensorDataConsistencies++;
        cout<<"GLOBALSTATISTICS: numSensorDataConsistencies-> "<<numSensorDataConsistencies<<" numSensorDataInconsistencies-> "<<numSensorDataInconsistencies<<" numInvalidSensorDataArrays-> "<<numInvalidSensorDataArrays<<endl;
    }
}

void GlobalStatistics::notifyOrchestrator(int i,int j, int vote){
    //If node i votes again for j as a normal node the message is ignored.
    int k=ANOMALOUS;
    if(vote == k)
        voteMatrix[i][j] = ANOMALOUS;
    int counter=0;
    for(int l=0;l<numSimNodes;l++){
        if(voteMatrix[l][j] == k)
            counter++;
    }
    //TODO DELETE NODE J

    if (counter>numSimNodes/2)
        cout<<"DELETING NODE "<<j<<endl;
}


void GlobalStatistics::handleMessage(cMessage* msg)
{
    if (msg == addMaliciousNodeTimer) {
        // schedule next timer event
        scheduleAt(simTime() + addMaliciousNodePeriod, msg);
        if(numMaliciousNodes<targetNumMaliciousNodes){
            addMaliciousNode();
            numMaliciousNodes++;
        }
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
    int controlledFlowDataCounter=0;
            for(int i=0;i<numSimNodes;i++)
                for(int j=0;j<numSimNodes;j++)
                    if(controlledFlowData[i][j]==true){
                        stringstream ssi;
                        ssi << i;
                        string str_i = ssi.str();
                        stringstream ssj;
                        ssj << j;
                        string str_j = ssj.str();
                        string s="node "+str_i+" controls flow data of node"+str_j;
                        recordScalar(s.c_str(),true);
                        controlledFlowDataCounter++;
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
    recordScalar("numDeliveredDataByMalicious", numDeliveredDataByMalicious);
    recordScalar("numDeliveredDataByNonMalicious", numDeliveredDataByNonMalicious);
/*
    recordScalar("percentage of controlled sensor data", numMaliciousNodes + controlledSensorDataCounter);
    recordScalar("percentage of fixed portion of controlled sensor data", numMaliciousNodes );
    recordScalar("percentage of variable portion of controlled sensor data", controlledSensorDataCounter);
    recordScalar("numSensorDataConsistencies", numSensorDataConsistencies);
    recordScalar("numSensorDataInconsistencies", numSensorDataInconsistencies);
*/
    recordScalar("invalidSensorDataPercentageMean", (double)invalidSensorDataPercentageMean.accumulator/invalidSensorDataPercentageMean.counter);
    //recordScalar("numInvalidSensorDataArrays", numInvalidSensorDataArrays);
    recordScalar("GlobalStatistics: Simulation Time", simTime());

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

