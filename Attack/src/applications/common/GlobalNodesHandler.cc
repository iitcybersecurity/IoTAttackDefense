

#include <omnetpp.h>

#include <GlobalStatisticsAccess.h>


#include "GlobalNodesHandler.h"

using namespace std;

Define_Module(GlobalNodesHandler);


GlobalNodesHandler::GlobalNodesHandler()
{
    periodicTimer = NULL;
}

GlobalNodesHandler::~GlobalNodesHandler()
{
    cancelAndDelete(periodicTimer);
}

void GlobalNodesHandler::initialize()
{
    p2pnsNameCount = 0;
    globalStatistics = GlobalStatisticsAccess().get();

    numSimNodes = par("targetOverlayTerminalNum");
    numDevices = par("numDevices");
    numSybils = numSimNodes - numDevices;
    numTemperatureDevices=par("numTemperatureDevices");
       numMotionDevices=par("numMotionDevices");
       numMaliciousNodes = par("numMaliciousNodes");
       targetNumMaliciousNodes = par("targetNumMaliciousNodes");
       numVictimNodes = par("numVictimNodes");
       groupA_nodesNumber = par("groupA_nodesNumber");
       groupB_nodesNumber = par("groupB_nodesNumber");
       classificationSlotTime = par("classificationSlotTime");
       scenario = par("scenario");
       application = par("application");	 
       nodes_counter=-1;




       //Create an entry for each node of the network, storing its overlay identifier
           LUT_nodesKeys.reserve(numSimNodes * 2);
           for (int i = 0; i < numSimNodes * 2; ++i) {
              std::stringstream ss;
              ss << "node" << i;
              std::string node = ss.str();
              BinaryValue binValue(node);
              LUT_nodesKeys[i] = OverlayKey::sha1(binValue);
           }
    cout<<"GlobalNodesHandler2"<<endl;  //Debug


    maliciousNodes=new bool[numDevices];
    victimNodes = new bool[numDevices];
    groupA_nodes = new bool[numDevices];
    groupB_nodes = new bool[numDevices];
    for(int i=0;i<numDevices;i++){
        maliciousNodes[i]=false;
        victimNodes[i]=false;
        groupA_nodes[i]=false;
        groupB_nodes[i]=false;
    }

    sybilNodes = new int[numSybils];
    temperatureDevices=new int[numTemperatureDevices];
    motionDevices=new int[numMotionDevices];

    isInvalidClassificationDataArray=new bool[numDevices]();
    for(int i=0;i<numDevices;i++){
        isInvalidClassificationDataArray[i]=false;
    }
    isInvalidClassificationResultArray=new bool*[numDevices]();
    for(int i=0;i<numDevices;i++){
        isInvalidClassificationResultArray[i]=new bool[numDevices]();
        for(int j=0;j<numDevices;j++)
            isInvalidClassificationResultArray[i][j]=false;
    }

     setMaliciousNodes(numMaliciousNodes);
    setVictimNodes(numVictimNodes);
    setGroupA_Nodes(groupA_nodesNumber);
    setGroupB_Nodes(groupB_nodesNumber);
    setSybils(numSybils);

    setTemperatureAndMotionDevices(numTemperatureDevices,numMotionDevices);

    cout<<"GlobalNodesHandler: victim nodes -> ";
    for(int i=0;i<numDevices;i++)
        if(victimNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: initial malicious nodes -> ";
    for(int i=0;i<numDevices;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: initial group A nodes -> ";
    for(int i=0;i<numDevices;i++)
        if(groupA_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: initial group B nodes -> ";
    for(int i=0;i<numDevices;i++)
        if(groupB_nodes[i])
            cout<<" "<<i;
    cout<<endl;

    cout<<"GlobalNodesHandler: temperature nodes -> ";
    for(int i=0;i<numTemperatureDevices;i++)
        cout<<" "<<temperatureDevices[i];
    cout<<endl;
    cout<<"GlobalNodesHandler: motion nodes -> ";
        for(int i=0;i<numMotionDevices;i++)
            cout<<" "<<motionDevices[i];
    cout<<endl;

    // start periodic globalStatTimer
    addMaliciousNodePeriod = par("addMaliciousNodePeriod");
    if (addMaliciousNodePeriod > 0) {
        addMaliciousNodeTimer = new cMessage("addMaliciousNode");
        scheduleAt(simTime() + 310, addMaliciousNodeTimer);  //DEBUG  we add the new malicious node after the first reputation_update
    }

}
void GlobalNodesHandler::handleMessage(cMessage* msg)
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
    error("GlobalNodesHandler::handleMessage(): Unknown message type!");
}
void GlobalNodesHandler::addMaliciousNode(){
    int array[numDevices];
    for(int i=0;i<numDevices-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numDevices-1);
    int i=0;
    while(isMalicious(array[i]))   //I choose a random node that is not already malicious.
        i++;
    maliciousNodes[array[i]]=true;
    cout<<"Node"<<array[i]<<" is now malicious!"<<endl; //debug
    cout<<"GlobalNodesHandler: actual victim nodes -> ";  //debug
    for(int i=0;i<numDevices;i++)
        if(victimNodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: actual malicious nodes -> ";   //debug
    for(int i=0;i<numDevices;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
    removeVictimNode(array[i]);    //I remove that node from the victim nodes list.
}
void GlobalNodesHandler::removeVictimNode(int victimNodeId){
    victimNodes[victimNodeId] = false;
}
void GlobalNodesHandler::removeGroupANode(int nodeId){
    groupA_nodes[nodeId] = false;
}

bool GlobalNodesHandler::isMalicious(int nodeId){
    return maliciousNodes[nodeId];
}
bool GlobalNodesHandler::isSybil(int nodeId){
    for(int i=0;i<numSybils;i++)
        if(sybilNodes[i]==nodeId)
            return true;
    return false;
}
bool GlobalNodesHandler::isTemperatureDevice(int nodeId){
    for(int i=0;i<numTemperatureDevices;i++)
        if(temperatureDevices[i]==nodeId)
            return true;
    return false;
}
bool GlobalNodesHandler::isMotionDevice(int nodeId){
    for(int i=0;i<numMotionDevices;i++)
            if(motionDevices[i]==nodeId)
                return true;
        return false;
}

//This function is called for setting the victims of the IDS-based attack.
//Used in scenario 6.1
void GlobalNodesHandler::setVictimNodes(int numVictimNodes){
    int array[numDevices];
    for(int i=0;i<numDevices;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numDevices);
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
void GlobalNodesHandler::addToGroupB(){

    int array[numDevices];
    for(int i=0;i<numDevices-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numDevices-1);
    int i=0;
    while(isMalicious(array[i]) || isGroupBNode(array[i]))   //I choose a random node that is not already malicious.
        i++;
    groupB_nodes[array[i]]=true;
    removeGroupANode(array[i]);    //I remove that node from the victim nodes list.

    cout<<"Node"<<array[i]<<" is being added to group B!"<<endl; //debug
    cout<<"GlobalNodesHandler: actual group B nodes -> ";  //debug
    for(int i=0;i<numDevices;i++)
        if(groupB_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: actual group A nodes -> ";   //debug
    for(int i=0;i<numDevices;i++)
        if(groupA_nodes[i])
            cout<<" "<<i;
    cout<<endl;
    cout<<"GlobalNodesHandler: actual malicious nodes -> ";   //debug
    for(int i=0;i<numDevices;i++)
        if(maliciousNodes[i])
            cout<<" "<<i;
    cout<<endl;
}
bool GlobalNodesHandler::isGroupANode(int nodeId){
    return groupA_nodes[nodeId];
}
bool GlobalNodesHandler::isGroupBNode(int nodeId){
    return groupB_nodes[nodeId];
}
void GlobalNodesHandler::setGroupA_Nodes(int groupA_nodesNumber){
    int array[numDevices];
    for(int i=0;i<numDevices;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numDevices);
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
void GlobalNodesHandler::setGroupB_Nodes(int groupB_nodesNumber){
    int array[numDevices];
    for(int i=0;i<numDevices;i++){
        array[i]=i;
    }
    random_shuffle(array, array+numDevices);
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

bool GlobalNodesHandler::isVictim(int nodeId){
    return victimNodes[nodeId];
}


void GlobalNodesHandler::setMaliciousNodes(int numMaliciousNodes){

    int array[numDevices];
    for(int i=0;i<numDevices-1;i++){
        array[i]=i+1;
    }
    random_shuffle(array, array+numDevices-1);
    for(int i=0;i<numMaliciousNodes;i++)
        maliciousNodes[array[i]]=true;
}
void GlobalNodesHandler::setSybils(int numSybils){
    for(int i=numDevices;i<numSimNodes;i++)
        sybilNodes[i-numDevices]=i;
}

void GlobalNodesHandler::setTemperatureAndMotionDevices(int numTemperatureDevices, int numMotionDevices){

    for(int i=0;i<numTemperatureDevices;i++)
        temperatureDevices[i]=i;
    for(int j=0;j<numMotionDevices;j++)
        motionDevices[j]=numTemperatureDevices+j;
}

int GlobalNodesHandler::getNumTemperatureDevices(){
    return numTemperatureDevices;
}
int GlobalNodesHandler::getNumMotionDevices(){
    return numMotionDevices;
}

int* GlobalNodesHandler::getTemperatureDevices(){
    return temperatureDevices;
}

int* GlobalNodesHandler::getMotionDevices(){
    return motionDevices;
}

int GlobalNodesHandler::getNumSybils(){
    return numSybils;
}
int * GlobalNodesHandler::getSybils(){
    return sybilNodes;
}

void GlobalNodesHandler::deleteApplicationNode()
{
    LifetimeChurn* l = check_and_cast < LifetimeChurn * > (underlayConfigurator->getChurnGenerator(0));
    l->deleteNode(*(l->nodesList[20]),99);

    //TODO delete node from malicious nodes and victim nodes
}

int GlobalNodesHandler::generateNodeId(){
    nodes_counter++;
    return nodes_counter;
}
