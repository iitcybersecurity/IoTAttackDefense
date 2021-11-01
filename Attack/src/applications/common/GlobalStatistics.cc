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
       groupA_nodesNumber = par("groupA_nodesNumber");
       groupB_nodesNumber = par("groupB_nodesNumber");
       classificationSlotTime = par("classificationSlotTime");
       scenario = par("scenario");

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


    maliciousNodes=new bool[numSimNodes];
    victimNodes = new bool[numSimNodes];
    groupA_nodes = new bool[numSimNodes];
    groupB_nodes = new bool[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        maliciousNodes[i]=false;
        victimNodes[i]=false;
        groupA_nodes[i]=false;
        groupB_nodes[i]=false;
    }
    temperatureDevices=new int[numTemperatureDevices];
    motionDevices=new int[numMotionDevices];

    voteMatrix= new int*[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        voteMatrix[i]=new int[numSimNodes];
        for(int j=0;j<numSimNodes;j++)
            voteMatrix[i][j]=NORMAL;
    }
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
    setGroupA_Nodes(groupA_nodesNumber);
    setGroupB_Nodes(groupB_nodesNumber);

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
    cout<<"GlobalStatistics: initial group A nodes -> ";
    for(int i=0;i<numSimNodes;i++)
        if(groupA_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: initial group B nodes -> ";
    for(int i=0;i<numSimNodes;i++)
        if(groupB_nodes[i])
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

    if(scenario == 51 || scenario == 52){
        //The voting mechanism must be scheduled for each slot time.
        InfoMessage* votes_count_timer = new InfoMessage("votes_count_timer");
        scheduleAt(simTime() + classificationSlotTime, votes_count_timer);
    }
    if(scenario == 61){ //In scenario 6.1 we evaluate the mean trust of malicious & non-malicious nodes
        InfoMessage* average_trust_evaluation_timer = new InfoMessage("average_trust_evaluation_timer");
        scheduleAt(simTime() + classificationSlotTime, average_trust_evaluation_timer);
    }
    if(scenario == 71 || scenario == 72){
        InfoMessage* average_trust_evaluation_timer = new InfoMessage("average_trust_evaluation_timer");
        scheduleAt(simTime() + classificationSlotTime, average_trust_evaluation_timer);
    }

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
void GlobalStatistics::removeGroupANode(int nodeId){
    groupA_nodes[nodeId] = false;
}

//This function is called for setting the victims of the IDS-based attack.
//Used in scenario 6.1
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
void GlobalStatistics::addToGroupB(){

    int array[numSimNodes];
    for(int i=0;i<numSimNodes-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numSimNodes-1);
    int i=0;
    while(isMalicious(array[i]) || isGroupBNode(array[i]))   //I choose a random node that is not already malicious.
        i++;
    groupB_nodes[array[i]]=true;
    removeGroupANode(array[i]);    //I remove that node from the victim nodes list.

    cout<<"Node"<<array[i]<<" is being added to group B!"<<endl; //debug
    cout<<"GlobalStatistics: actual group B nodes -> ";  //debug
    for(int i=0;i<numSimNodes;i++)
        if(groupB_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: actual group A nodes -> ";   //debug
    for(int i=0;i<numSimNodes;i++)
        if(groupA_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalStatistics: actual malicious nodes -> ";   //debug
    for(int i=0;i<numSimNodes;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
}
bool GlobalStatistics::isGroupANode(int nodeId){
    return groupA_nodes[nodeId];
}
bool GlobalStatistics::isGroupBNode(int nodeId){
    return groupB_nodes[nodeId];
}
void GlobalStatistics::setGroupA_Nodes(int groupA_nodesNumber){
    int array[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numSimNodes);
    int j=0;
    int i=0;
    while(j<groupA_nodesNumber){
        if(!isMalicious(array[i]) && !isGroupBNode(array[i])){
            groupA_nodes[array[i]] = true;
            j++;
        }
        i++;
    }
}
void GlobalStatistics::setGroupB_Nodes(int groupB_nodesNumber){
    int array[numSimNodes];
    for(int i=0;i<numSimNodes;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numSimNodes);
    int j=0;
    int i=0;
    while(j<groupB_nodesNumber){
        if(!isMalicious(array[i]) && !isGroupANode(array[i])){
            groupB_nodes[array[i]] = true;
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
    if(dataType == "classification_data")
        controlledClassificationData[maliciousNode][victimNode]=true;
    if(dataType == "classification_result")
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
void GlobalStatistics::registerTrust(int nodeId,int trust){
    if(isMalicious(nodeId)){
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
    if (msg == addMaliciousNodeTimer) {
        // schedule next timer event
        scheduleAt(simTime() + addMaliciousNodePeriod, msg);
        if(numMaliciousNodes<targetNumMaliciousNodes){
            addMaliciousNode();
            numMaliciousNodes++;
        }
        return;
    }
    InfoMessage * myMsg = dynamic_cast < InfoMessage * > (msg);
    if (myMsg -> isName("addToGroupB")) {
            // schedule next timer event
            scheduleAt(simTime() + classificationSlotTime, msg);
            if(groupB_nodesNumber<numSimNodes/2){
                addToGroupB();
            }
            return;
    }
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
           for(int l=0;l<numSimNodes;l++){
               for(int i=0;i<numSimNodes;i++){
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
    recordScalar("numDeliveredDataByMalicious", numDeliveredDataByMalicious);
    recordScalar("numDeliveredDataByNonMalicious", numDeliveredDataByNonMalicious);
    recordScalar("numDeliveredDataByNonMalicious", numDeliveredDataByNonMalicious);

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

