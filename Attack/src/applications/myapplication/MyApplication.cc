
#include <IPAddressResolver.h>
#include <TransportAddress.h>
#include <GlobalNodeListAccess.h>
#include <GlobalStatisticsAccess.h>
#include <UnderlayConfiguratorAccess.h>
#include <LifetimeChurn.h>
#include <RpcMacros.h>
#include "CommonMessages_m.h"
 //
//#include "c_tokenizer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include "UnderlayConfigurator.h"
#include <UnderlayConfiguratorAccess.h>
#include "GlobalStatistics.h"
#include "MyMessage_m.h"
#include "MyApplication.h"

Define_Module(MyApplication);


// initializeApp() is called when the module is being created.
// Use this function instead of the constructor for initializing variables.
void MyApplication::initializeApp(int stage)
{
    // initializeApp will be called twice, each with a different stage.
    // stage can be either MIN_STAGE_APP (this module is being created),
    // or MAX_STAGE_APP (all modules were created).
    // We only care about MIN_STAGE_APP here.

    if (stage != MIN_STAGE_APP) return;

    // copy the module parameter values to our own variables
    numReplicas = par("numReplicas");
    numSimNodes = par("numSimNodes");
    flowSlotTime = par("flowSlotTime");
    //flowPerSlot = par("flowPerSlot");
    topology=par("topology");
    putTemperaturePeriod = par("putTemperaturePeriod");
    readTemperatureProbability = par("readTemperatureProbability");
    readMotionProbability = par("readMotionProbability");
    /*w = par("w");
    r = par("r");
    t = par("t");
    u = par("u");
    d = par("d");   */
    trustThreshold = par("trustThreshold");
    reputationThreshold = par("reputationThreshold");
    beliefVar = par("beliefVar");
    disbeliefVar = par("disbeliefVar");
    reputationStrikesLimit = par("reputationStrikesLimit");
    //Attacks
    resourceExhaustionAttack = par("resourceExhaustionAttack");
    invalidDataAttack = par("invalidDataAttack");
    scenario5Attack = par("scenario5Attack");
    isDataIntegrityEnabled = par("isDataIntegrityEnabled");

    globalNodeList = GlobalNodeListAccess().get();
    underlayConfigurator = UnderlayConfiguratorAccess().get();
    globalStatistics = GlobalStatisticsAccess().get();

    // statistics
    numSent = 0;
    numGetSent = 0;
    numPutSent = 0;
    simResetTime = 20;

    //generate nodeId of this node
    nodeId = generateNodeId();
    // initialize our statistics variables
    numSent = 0;
    numReceived = 0;

    //the array that will contain topology1 sensor data must be initialized
    sensorDataArray = new char[numSimNodes]();
     for(int i=0;i<numSimNodes;i++){
        sensorDataArray[i]='2'; //it means that the i-th node's detected value hasn't been received yet
     }
     sampleCounter = 0;
     sensorData = new string[N];
     for(int i=0;i<N;i++){
         sensorData[i]="empty";
     }

     //the array that will contain my entire flow data must be initialized
     flowSlotNumber = -1;
     flowData = new string[N];
     for(int i=0;i<N;i++){
         flowData[i]="empty";
     }
     //the array that will contain current flow data of other nodes  must be initialized
     flowDataArray = new string*[numSimNodes]();
     for(int i=0;i<numSimNodes;i++){
         flowDataArray[i]=new std::string[10];
         for(int j=0;j<10;j++){
             flowDataArray[i][j]="empty";
         }
     }
     //The reputation of a node is distributed, meaning that i must collect the votes of the entire network and evaluate the validity of each possible outcome.
     reputation = new double*[numSimNodes]();
     for(int i=0;i<numSimNodes;i++){
         reputation[i]= new double[3];
         reputation[i][0] = 1;  //Validity of normal behavior
         reputation[i][1] = 1;  //Validity of malicious behavior
         reputation[i][2] = 0;  //number of strikes. at 3 strikes a node is considered as anomalous.
     }
     //at beginning each node of the network is considered as trustful
     trust = new double*[numSimNodes]();
      for(int i=0;i<numSimNodes;i++){
          trust[i]= new double[2] ;
          trust[i][0] = 1;  //belief
          trust[i][1] = 0;   //disbelief
      }
      //each entry of voteMatrix must be initializes with NOT_AVAILALE.
      voteMatrix = new double*[numSimNodes]();
        for(int i=0;i<numSimNodes;i++){
            voteMatrix[i]= new double[numSimNodes]();
            for(int j=0;j<numSimNodes;j++){
                voteMatrix[i][j]= NOT_AVAILABLE;
            }
        }


    LUT_nodesKeys.reserve(numSimNodes * 2);
    for (int i = 0; i < numSimNodes * 2; ++i) {

       std::stringstream ss;
       ss << "node" << i;
       std::string node = ss.str();
       BinaryValue binValue(node);
       LUT_nodesKeys[i] = OverlayKey::sha1(binValue);
    }
    std::cout << "node" << nodeId <<", ip => "<< thisNode.getIp() << ", key => " << LUT_nodesKeys[nodeId] << endl;

    // These are messages that must be scheduled and handled by handleTimerMessage()
    dhttestput_timer = new InfoMessage("topology1_put_timer");
    dhttestget_timer = new InfoMessage("topology1_get_timer");
    put_temperature_timer = new InfoMessage("put_temperature_timer");
    get_temperature_timer = new InfoMessage("get_temperature_timer");
    put_motion_timer = new InfoMessage("put_motion_timer");
    get_motion_timer = new InfoMessage("get_motion_timer");
    put_flow_timer = new InfoMessage("put_flow_timer");
    get_flow_timer = new InfoMessage("get_flow_timer");
    topology1_classification_timer = new InfoMessage("topology1_classification_timer");
    flow_classification_timer = new InfoMessage("flow_classification_timer");
    get_vote_timer = new InfoMessage("get_vote_timer");
    update_reputation_timer = new InfoMessage("update_reputation_timer");
    UDPsend_timer = new InfoMessage("UDPsend_timer");


    fstream newfile;
    string str;
    //open the file to read my topology1 sensor data
    newfile.open("onlySensors.txt", ios:: in );
    if (newfile.is_open()) {
        getline(newfile, str); //initial row must be discharged
        int i=0;
        while (getline(newfile, str) && i<N) {
            str.erase(std::remove(str.begin(), str.end(), ','), str.end());
            sensorData[i] = string(1,str[nodeId]);
            i++;
        }
    }
    newfile.close();

    //open the file to read my network flow data
    string s="network_dataset_" + thisNode.getIp().str()+ ".txt";
    newfile.open(s.c_str(), ios:: in );
    if (newfile.is_open()) {
          getline(newfile, str); //initial row must be discharged
          int line_counter=0;
          while (getline(newfile, str) && line_counter!=N) {
              flowData[line_counter] = str;
              line_counter++;
          }
      }
    newfile.close();

    if(topology == 1){
        if(sensorData[0] != "empty")
            scheduleAt(simTime() + 15, dhttestput_timer);
        scheduleAt(simTime() + 30, dhttestget_timer);
        scheduleAt(simTime() + 31, topology1_classification_timer);
    }

    if(topology == 2){
        if(globalStatistics->isTemperatureDevice(nodeId)){
            scheduleAt(simTime() + 15, put_temperature_timer);
        }
        if(globalStatistics->isMotionDevice(nodeId)){
            scheduleAt(simTime() + 15, put_motion_timer);
        }
        scheduleAt(simTime() + 30 , get_temperature_timer);
        scheduleAt(simTime() + 30 , get_motion_timer);

    }
    if(topology == 3){  //flowSlotTime is divided in 3 subslots. we assume that PUT/GET operations can be completed within flowSlotTime/3 seconds
        if(flowData[0] != "empty")
            scheduleAt(simTime() + flowSlotTime, put_flow_timer);  //first at 100s
        scheduleAt(simTime() + flowSlotTime + (flowSlotTime/3), get_flow_timer); //first at 133s
        scheduleAt(simTime() + flowSlotTime + (flowSlotTime)*2/3, flow_classification_timer); //first at 166s
        scheduleAt(simTime() + flowSlotTime*2 , get_vote_timer); //first at 200s
        scheduleAt(simTime() + flowSlotTime*2 + flowSlotTime/2, update_reputation_timer); //first at 250s

    }
    bindToPort(2000);
}

void MyApplication::deleteApplicationNode()
{
    LifetimeChurn* l = check_and_cast < LifetimeChurn * > (underlayConfigurator->getChurnGenerator(0));
    //globalNodeList->killPeer(l->nodesList.at(5)->getAddress());
    //cout<<"size: "<<l->nodesList.size()<<endl;
    l->deleteNode(*(l->nodesList[20]),99);
}


void MyApplication::saveTopology1SensorData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    topology1SensorDataMap.erase(key);
    topology1SensorDataMap.insert(make_pair(key, entry));

}

void MyApplication::saveTemperatureData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    temperatureDataMap.erase(key);
    temperatureDataMap.insert(make_pair(key, entry));

   // DhtTestEntryTimer* msg = new DhtTestEntryTimer("dhtEntryTimer");
   // msg->setKey(key);

    //scheduleAt(entry.endtime, msg);
}
void MyApplication::saveMotionData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    motionDataMap.erase(key);
    motionDataMap.insert(make_pair(key, entry));

   // DhtTestEntryTimer* msg = new DhtTestEntryTimer("dhtEntryTimer");
   // msg->setKey(key);

    //scheduleAt(entry.endtime, msg);
}

void MyApplication::saveFlowData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    flowDataMap.erase(key);
    flowDataMap.insert(make_pair(key, entry));


}

void MyApplication::saveVoteData(const OverlayKey& key, const DHTDoubleEntry& entry)
{
    Enter_Method_Silent();

    voteDataMap.erase(key);
    voteDataMap.insert(make_pair(key, entry));
}


const DHTEntry* MyApplication::retrieveTopology1SensorData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = topology1SensorDataMap.find(key);

    if (it == topology1SensorDataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

const DHTEntry* MyApplication::retrieveTemperatureData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = temperatureDataMap.find(key);

    if (it == temperatureDataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

const DHTEntry* MyApplication::retrieveMotionData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = motionDataMap.find(key);

    if (it == motionDataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

const DHTEntry* MyApplication::retrieveFlowData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = flowDataMap.find(key);

    if (it == flowDataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}


const DHTDoubleEntry* MyApplication::retrieveVoteData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTDoubleEntry>::iterator it = voteDataMap.find(key);

    if (it == voteDataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

void MyApplication::handleTimerEvent(cMessage * msg) {

  InfoMessage * myMsg = dynamic_cast < InfoMessage * > (msg);
  if (myMsg == 0) {
    delete msg; // unknown!
    return;
  }

  if (myMsg -> isName("topology1_put_timer")) {
    try {

      for(int i=0;i<=numReplicas;i++){ //debug
          MyMessage *myMessage; // the message we'll send
          myMessage = new MyMessage();
          myMessage->setType(MYMSG_PUT_TOPOLOGY1_DATA); // set the message type
          myMessage->setTimeSlot(sampleCounter);
          std::stringstream ss;
          ss << "node" << nodeId <<"_sensorData_"<<i;
          std::string node = ss.str();
          BinaryValue binValue(node);
          myMessage->setKey(OverlayKey::sha1(binValue));
          myMessage->setOwnerNodeId(nodeId); //Set the id of the node that originated the data
          if(invalidDataAttack && globalStatistics->isMalicious(nodeId))
              //TODO the attacker must set an arbitrary sensor data value
              myMessage->setDetectedValue("3");
          else
              myMessage->setDetectedValue(sensorData[sampleCounter].c_str());
          myMessage->setSenderAddress(thisNode);
          callRoute(myMessage->getKey(), myMessage);
      }
      sampleCounter ++;
      if(sampleCounter == N || sensorData[sampleCounter] == "empty")
          return;
      scheduleAt(simTime() + 3, myMsg);
    }
    catch (...) {
          cout << "\n Insertion failed.";
          return;
        }
  }
  else if (myMsg -> isName("topology1_get_timer")) {
    for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {

          MyMessage *myMessage;
          myMessage = new MyMessage();
          myMessage->setType(MYMSG_GET_TOPOLOGY1_DATA); // set the message type

          std::stringstream ss;
          ss << "node" << i <<"_sensorData_"<<intuniform(0,numReplicas);  //debug
          std::string node = ss.str();
          BinaryValue binValue(node);
          myMessage->setKey(OverlayKey::sha1(binValue));
          myMessage->setRequesterNodeId(nodeId);
          myMessage->setOwnerNodeId(i);
          myMessage->setSenderAddress(thisNode);
          callRoute(OverlayKey::sha1(binValue), myMessage);
    }
    scheduleAt(simTime() + 3, myMsg);
  }

  if (myMsg -> isName("put_temperature_timer")){

      int temperatureValue=intuniform(1, 100);
      for(int i=0;i<=numReplicas;i++){

            MyMessage *myMessage;
            myMessage = new MyMessage();
            myMessage->setType(MYMSG_PUT_TEMPERATURE);
            std::stringstream ss;
            ss << "node" << nodeId <<"_temperatureData_"<<i;
            std::string node = ss.str();
            BinaryValue binValue(node);
            myMessage->setKey(OverlayKey::sha1(binValue));
            myMessage->setOwnerNodeId(nodeId);
            if(invalidDataAttack && globalStatistics->isMalicious(nodeId))
                myMessage->setTemperatureValue("-1"); //malicious value
            else{
                stringstream ss;
                ss << temperatureValue;
                string str = ss.str();
                myMessage->setTemperatureValue(str.c_str());
            }
            myMessage->setSenderAddress(thisNode);
            callRoute(myMessage->getKey(), myMessage);
      }
      scheduleAt(simTime() + putTemperaturePeriod, put_temperature_timer);
  }

  if (myMsg -> isName("put_motion_timer")){

        int motionValue=intuniform(0, 1);
        for(int i=0;i<=numReplicas;i++){
              MyMessage *myMessage;
              myMessage = new MyMessage();
              myMessage->setType(MYMSG_PUT_MOTION);
              std::stringstream ss;
              ss << "node" << nodeId <<"_motionData_"<<i;
              std::string node = ss.str();
              BinaryValue binValue(node);
              myMessage->setKey(OverlayKey::sha1(binValue));
              myMessage->setOwnerNodeId(nodeId);
              if(invalidDataAttack && globalStatistics->isMalicious(nodeId))
                  myMessage->setMotionValue("-1"); //malicious value
              else{
                  stringstream ss;
                  ss << motionValue;
                  string str = ss.str();
                  myMessage->setMotionValue(str.c_str());
              }
              myMessage->setSenderAddress(thisNode);
              callRoute(myMessage->getKey(), myMessage);
        }
        scheduleAt(simTime() + putTemperaturePeriod, put_motion_timer);
    }

  //each node reads data temperature/motion data from a responsible node with a given probability.
  if (myMsg -> isName("get_temperature_timer")){
      cout<<nodeId<<"get_temperature_timer"<<endl;

      int* temperatureDevices = globalStatistics -> getTemperatureDevices();
      for (unsigned int i = 0; i < globalStatistics -> getNumTemperatureDevices(); ++i) {
                if(readTemperatureProbability<intuniform(0,100))
                    continue;
                MyMessage *myMessage;
                myMessage = new MyMessage();
                myMessage->setType(MYMSG_GET_TEMPERATURE); // set the message type to PING

                std::stringstream ss;
                ss << "node" << temperatureDevices[i] <<"_temperatureData_"<<intuniform(0,numReplicas);
                std::string node = ss.str();
                BinaryValue binValue(node);
                myMessage->setKey(OverlayKey::sha1(binValue));
                myMessage->setOwnerNodeId(temperatureDevices[i]);
                myMessage->setRequesterNodeId(nodeId);
                myMessage->setSenderAddress(thisNode);
                callRoute(OverlayKey::sha1(binValue), myMessage);
          }

      scheduleAt(simTime() + intuniform(10, 100), get_temperature_timer);

  }

  //each node reads data temperature/motion data from a responsible node with a given probability.
    if (myMsg -> isName("get_motion_timer")){
        cout<<nodeId<<"get_motion_timer"<<endl;

        int* motionDevices = globalStatistics -> getMotionDevices();

        for (unsigned int i = 0; i < globalStatistics -> getNumMotionDevices(); ++i) {
                  if(readMotionProbability<intuniform(0,100))
                      continue;
                  MyMessage *myMessage;
                  myMessage = new MyMessage();
                  myMessage->setType(MYMSG_GET_MOTION); // set the message type

                  std::stringstream ss;
                  ss << "node" << motionDevices[i] <<"_motionData_"<<intuniform(0,numReplicas);
                  std::string node = ss.str();
                  BinaryValue binValue(node);
                  myMessage->setKey(OverlayKey::sha1(binValue));
                  myMessage->setOwnerNodeId(motionDevices[i]);
                  myMessage->setRequesterNodeId(nodeId);
                  myMessage->setSenderAddress(thisNode);
                  callRoute(OverlayKey::sha1(binValue), myMessage);
            }
        scheduleAt(simTime() + intuniform(10, 100), get_motion_timer);
    }


  if (myMsg -> isName("topology1_classification_timer")) {
      //TODO classification of sensorDataArray

      scheduleAt(simTime() + 3, myMsg);
  }

  if (myMsg -> isName("put_flow_timer")) {
      try {
          flowSlotNumber++;
          for(int i=0;i<numSimNodes;i++){  //One replica for each other node
                if(i == nodeId)
                    continue;
                MyMessage *myMessage;
                myMessage = new MyMessage();
                myMessage->setType(MYMSG_PUT_FLOW);
                std::stringstream ss;
                //ss << "node" << nodeId <<"_flow_vector_"<<flowSlotNumber;
                ss << "node" << nodeId <<"_flow_vector_replica"<<i;
                std::string node = ss.str();
                BinaryValue binValue(node);
                myMessage->setKey(OverlayKey::sha1(binValue));
                myMessage->setOwnerNodeId(nodeId);
                int index=flowSlotNumber*10;     //We consider 10 flows inside the flow classification data for each slot time
                for(int j=0;j<10;j++){
                    myMessage->setFlowData(j, flowData[index + j].c_str());
                }
                myMessage->setSenderAddress(thisNode);
                callRoute(myMessage->getKey(), myMessage);
          }

            int index = (flowSlotNumber+1)*10;
            if(flowData[index] == "empty"){  //if next slot's flow data is not available then i stop
                 return;
            }
            scheduleAt(simTime() + flowSlotTime, myMsg);
      }
      catch (...) {
            cout << "\n Insertion failed.";
            return;
          }
    }

  if (myMsg -> isName("get_flow_timer")){
      for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
                if(i==nodeId)
                    continue;
                MyMessage *myMessage;
                myMessage = new MyMessage();
                myMessage->setType(MYMSG_GET_FLOW); // set the message type

                std::stringstream ss;
                //I get only my own copy of node i's flow data
                ss << "node" << i <<"_flow_vector_replica"<<nodeId;
                std::string node = ss.str();
                BinaryValue binValue(node);
                myMessage->setKey(OverlayKey::sha1(binValue));
                myMessage->setOwnerNodeId(i);
                myMessage->setRequesterNodeId(nodeId);
                myMessage->setSenderAddress(thisNode);
                callRoute(OverlayKey::sha1(binValue), myMessage);
       }
       scheduleAt(simTime() + flowSlotTime, myMsg);
   }
  if (myMsg -> isName("flow_classification_timer")) {

          for(int i=0;i<numSimNodes;i++){

              if(i!=nodeId){

              if(globalStatistics->isMalicious(nodeId)){
                  if(globalStatistics->isVictim(i)){
                      //cout<<nodeId<<" voting against "<<i<<endl;
                      voteMatrix[nodeId][i]= ANOMALOUS ;
                  }
                  // if me an node i are both malicious nodes then i classify it as normal
                  if(globalStatistics->isMalicious(i)){
                      voteMatrix[nodeId][i]= NORMAL ;
                  }
                }
                  else{
                      //TODO CLASSIFICAZIONE DI flowDataArray[i][j]
                      //if im not a malicious node and node i is not having an anomalous behavior
                      voteMatrix[nodeId][i]= NORMAL ;
                  }
                }
          }

          //If we are considering a distributed reputation model we must insert into the dht the result of our classification
          for(int i=0;i<numSimNodes;i++){
              if(i!= nodeId){
                  MyMessage *myMessage = new MyMessage();
                    myMessage->setType(MYMSG_PUT_VOTE);
                   // myMessage->setTimeSlot(sampleCounter);
                    std::stringstream ss;
                    ss << "node" << nodeId <<"_voteData_"<<i;
                    std::string node = ss.str();
                    BinaryValue binValue(node);
                    myMessage->setKey(OverlayKey::sha1(binValue));
                    myMessage->setOwnerNodeId(nodeId);
                    myMessage->setTargetNodeId(i);
                    myMessage->setVoteValue(voteMatrix[nodeId][i]);
                    myMessage->setSenderAddress(thisNode);
                    callRoute(myMessage->getKey(), myMessage);
              }
          }

     scheduleAt(simTime() + flowSlotTime, myMsg);
    }

  if (myMsg -> isName("get_vote_timer")){
        for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
            for (unsigned int j = 0; j < globalNodeList -> getNumNodes(); ++j) {

                if(i!=nodeId && j!= nodeId && i!=j){
                      MyMessage *myMessage = new MyMessage();
                      myMessage->setType(MYMSG_GET_VOTE);

                      std::stringstream ss;
                      ss << "node" << i <<"_voteData_"<<j;

                      std::string node = ss.str();
                      BinaryValue binValue(node);
                      myMessage->setKey(OverlayKey::sha1(binValue));
                      myMessage->setRequesterNodeId(nodeId);
                      myMessage->setOwnerNodeId(i);
                      myMessage->setTargetNodeId(j);
                      myMessage->setSenderAddress(thisNode);
                      callRoute(OverlayKey::sha1(binValue), myMessage);

                }
            }
        }
         scheduleAt(simTime() + flowSlotTime, myMsg);
     }

  if (myMsg -> isName("update_reputation_timer")){
         // cout << "Node: " << nodeId << ", update_reputation_timer."<< endl;
          if(!globalStatistics->isMalicious(nodeId)){
              for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
                  double anomalousValidity = 0;
                  double normalValidity = 0;
                  double k= ANOMALOUS;
                  if(voteMatrix[nodeId][i] == k){
                      anomalousValidity= anomalousValidity +1;
                  }
                  else{
                      normalValidity = normalValidity+1;
                  }

                  for (unsigned int j = 0; j < globalNodeList -> getNumNodes(); ++j) {
                      if(i!=nodeId && j!= nodeId && i!=j){
                          if(trust[j][0]-trust[j][1] >= trustThreshold){  //belief - disbelief > threshold?
                              int anomalous = ANOMALOUS;
                              double normal = NORMAL;
                              double not_available= NOT_AVAILABLE;
                              if(voteMatrix[j][i] == anomalous)
                                  anomalousValidity = anomalousValidity + trust[j][0]-trust[j][1];
                              if(voteMatrix[j][i] == normal)
                                  normalValidity = normalValidity + trust[j][0]-trust[j][1];
                              if(voteMatrix[j][i] == not_available && nodeId == 0){  //debug
                                  cout<<"node "<<nodeId<<"didn't get vote of "<<j<<" to "<<i<<endl;
                              }
                              if(voteMatrix[j][i] != not_available && voteMatrix[j][i] != anomalous && voteMatrix[j][i] != normal && nodeId == 0){  //debug
                                cout<<"invalid vote: "<<j<<" to "<<i<<" vote is "<<voteMatrix[j][i]<<endl;

                              }
                          }

                      }
                  }
                  reputation[i][0] = normalValidity;
                  reputation[i][1] = anomalousValidity;

                  double anomalous= ANOMALOUS;
                  double normal = NORMAL;
                  int finalResult = ((reputation[i][0]/(reputation[i][0]+reputation[i][1])) >= reputationThreshold)?normal:anomalous;
                  //After we classify node i as normal or anomalous we update the trust of the nodes before proceeding with the next iteration
                  for(int j=0;j<numSimNodes;j++){
                    if(nodeId!=j && nodeId!= i && i!=j){
                        if(finalResult == voteMatrix[j][i]){
                            trust[j][0]+= beliefVar;
                            trust[j][1]-= beliefVar;
                            if(trust[j][0] > 1){   //trust components must be kept within range {0,1}
                                trust[j][0] = 1;
                                trust[j][1] = 0;
                            }
                        }
                        else{
                            trust[j][0]-= disbeliefVar;
                            trust[j][1]+= disbeliefVar;
                            if(trust[j][0] < 0){   //trust components must be kept within range {0,1}
                                 trust[j][0] = 0;
                                 trust[j][1] = 1;
                            }
                        }
                    }

                  }
              }
              //UDPATE TRUST
              /*
              for(int i=0;i<numSimNodes;i++){
                  double anomalous= ANOMALOUS;
                  double normal = NORMAL;
                  int finalResult = ((reputation[i][0]/(reputation[i][0]+reputation[i][1])) >= reputationThreshold)?normal:anomalous;
                  for(int j=0;j<numSimNodes;j++){
                      if(nodeId!=j && nodeId!= i && i!=j){
                          if(finalResult == voteMatrix[j][i]){
                              trust[j][0]+= beliefVar;
                              trust[j][1]-= beliefVar;
                              if(trust[j][0] > 1){   //trust components must be kept within range {0,1}
                                  trust[j][0] = 1;
                                  trust[j][1] = 0;
                              }
                          }
                          else{
                              trust[j][0]-= disbeliefVar;
                              trust[j][1]+= disbeliefVar;
                              if(trust[j][0] < 0){   //trust components must be kept within range {0,1}
                                   trust[j][0] = 0;
                                   trust[j][1] = 1;
                              }
                          }
                      }

                  }
              }*/

              for(int i=0;i<numSimNodes;i++)//debug
                  if(i!=nodeId && nodeId==0){
                      cout<<nodeId<<" "<<i<<" reputation: "<<reputation[i][0]<<" , "<<reputation[i][1]<<"   mean:"<<reputation[i][0]/(reputation[i][0]+reputation[i][1])<<"  trust:"<<trust[i][0]<<", "<<trust[i][1]<<endl;
                      if(reputation[i][0]/(reputation[i][0]+reputation[i][1]) < reputationThreshold){
                              reputation[i][2]++;
                              cout<<"node"<< i <<" has "<<reputation[i][2]<<" strike/s..."<<endl;
                              if(reputation[i][2] == reputationStrikesLimit)
                                  cout<<"node"<< i <<" is malicious!"<<endl;
                      }
                  }
              for(int i=0;i<numSimNodes;i++){  //RESET VOTES
                  for(int j=0;j<numSimNodes;j++){
                    voteMatrix[i][j] = NOT_AVAILABLE;
                }
              }
          }

        scheduleAt(simTime() + flowSlotTime, myMsg);
  }

}


// finish is called when the module is being destroyed
void MyApplication::finishApp(){
    // finish() is usually used to save the module's statistics.
    // We'll use globalStatistics->addStdDev(), which will calculate min, max, mean and deviation values.
    // The first parameter is a name for the value, you can use any name you like (use a name you can find quickly!).
    // In the end, the simulator will mix together all values, from all nodes, with the same name.

    globalStatistics->addStdDev("MyApplication: Sent packets", numSent);
    globalStatistics->addStdDev("MyApplication: Received packets", numReceived);
}



// deliver() is called when we receive a message from the overlay.
// Unknown packets can be safely deleted here.
void MyApplication::deliver(OverlayKey& key, cMessage* msg)
{
    // we are only expecting messages of type MyMessage, throw away any other
    MyMessage *myMsg = dynamic_cast<MyMessage*>(msg);
    if (myMsg == NULL) {
        cout<<"unknown message in deliver()"<<endl;
        delete msg; // type unknown!
        return;
    }

    if (myMsg->getType() == MYMSG_PUT_TOPOLOGY1_DATA) {
            //cout<<"node"<<nodeId<<": PUT request from node"<<myMsg->getOwnerNodeId()<<".Time: "<<myMsg->getTimeSlot()<<" key: "<<key.toString(16)<<endl;
        // if the responsible node is malicious and the owner of the temperature data is the victim then the attacker can manipulate the temperature data. (if integrity check is disabled)
            if(invalidDataAttack && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"topology1");
                myMsg->setDetectedValue("3");
            }
            bool isInvalidData;
            if(invalidDataAttack && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
               isInvalidData=true;
           }
           else
               isInvalidData=false;

           string* s=new string[1]();
           s[0]=myMsg->getDetectedValue();
           DHTEntry entry = {
                s,
                simTime(),
                isInvalidData
              };
            saveTopology1SensorData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
     }

    if (myMsg->getType() == MYMSG_GET_TOPOLOGY1_DATA) {
        //cout<<thisNode.getAddress()<<": GET request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" NodeId:"<<myMsg->getOwnerNodeId()<<endl;
        DHTEntry * entry = const_cast < DHTEntry * > (retrieveTopology1SensorData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
        if(entry == NULL){
            delete myMsg;
            cout<<"MYMSG_GET_TOPOLOGY1: DROPPING MESSAGE"<<endl;
            return;
        }
        MyMessage *response_msg = new MyMessage();
        response_msg->setType(MYMSG_GET_TOPOLOGY1_RESPONSE);
        response_msg->setDetectedValue(entry->value[0].c_str());
        response_msg->setIsInvalidData(entry->isInvalidData);
        response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
        response_msg->setSenderAddress(thisNode);
        sendMessageToUDP(myMsg->getSenderAddress(), response_msg);
    }

    if (myMsg->getType() == MYMSG_PUT_TEMPERATURE) {
    //        cout<<"node"<<nodeId<<": PUT_TEMPERATURE request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;
        // if the responsible node is malicious and the owner of the temperature data is the victim then the attacker can manipulate the temperature data. (if integrity check is disabled)
            if(invalidDataAttack && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"temperature");
            }
            bool isInvalidData;
            if(invalidDataAttack && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
                isInvalidData=true;
                //myMsg->setTemperatureValue("-1");
            }
            else
                isInvalidData=false;
            string* s=new string[1];
            s[0]= myMsg->getMotionValue();
            DHTEntry entry = {
                s,
                simTime(),
                isInvalidData
              };
            saveTemperatureData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
        }

    if (myMsg->getType() == MYMSG_GET_TEMPERATURE) {
   //        cout<<thisNode.getAddress()<<": GET_TEMPERATURE request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" NodeId:"<<myMsg->getOwnerNodeId()<<endl;
            DHTEntry * entry = const_cast < DHTEntry * > (retrieveTemperatureData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
            if(entry == NULL){
                delete myMsg;
                cout<<"MYMSG_GET_TEMPERATURE: DROPPING MESSAGE"<<endl;

                return;
            }
            MyMessage *get_response_msg = new MyMessage();
            get_response_msg->setType(MYMSG_GET_TEMPERATURE_RESPONSE);
            get_response_msg->setTemperatureValue(entry->value[0].c_str());
            get_response_msg->setIsInvalidData(entry->isInvalidData);
            get_response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
            get_response_msg->setSenderAddress(thisNode.getAddress());
            sendMessageToUDP(myMsg->getSenderAddress(), get_response_msg);
        }
    if (myMsg->getType() == MYMSG_PUT_MOTION) {
                //cout<<"node"<<nodeId<<": PUT_MOTION request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;
                // if the responsible node is malicious and the owner of the motion data is the victim then the attacker can manipulate the data. (if integrity check is disabled)
                if(invalidDataAttack && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                    globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"motion");
                }
                bool isInvalidData;
                if(invalidDataAttack && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
                    isInvalidData=true;
                }
                else
                    isInvalidData=false;
                string* s=new string[1];
                s[0]= myMsg->getMotionValue();
                DHTEntry entry = {
                    s,
                    simTime(),
                    isInvalidData
                  };
                saveMotionData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
     }

     if (myMsg->getType() == MYMSG_GET_MOTION) {
            cout<<thisNode.getAddress()<<": GET_MOTION request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" NodeId:"<<myMsg->getOwnerNodeId()<<endl;
            DHTEntry * entry = const_cast < DHTEntry * > (retrieveMotionData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
            if(entry == NULL){
                delete myMsg;
                cout<<"MYMSG_GET_MOTION: DROPPING MESSAGE"<<endl;

                return;
            }
            MyMessage *get_response_msg = new MyMessage();
            get_response_msg->setType(MYMSG_GET_MOTION_RESPONSE);
            get_response_msg->setMotionValue(entry->value[0].c_str());
            get_response_msg->setIsInvalidData(entry->isInvalidData);
            get_response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
            get_response_msg->setSenderAddress(thisNode.getAddress());
            sendMessageToUDP(myMsg->getSenderAddress(), get_response_msg);
      }
      if (myMsg->getType() == MYMSG_PUT_FLOW) {
          //cout<<"node"<<nodeId<<": PUT_FLOW request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;

     /*     if(scenario5Attack && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
              globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"flow");
          }*/
          bool isInvalidData;
          // if the responsible node is malicious and the owner of the flow data is the victim then the attacker can manipulate the flow data. (if integrity check is disabled)
          if(scenario5Attack && !isDataIntegrityEnabled && globalStatistics->isMalicious(nodeId) && globalStatistics->isVictim(myMsg->getOwnerNodeId())){
              isInvalidData=true;
              //TODO substitute normal flow data with anomalous data.
          }
          else
              isInvalidData=false;
          string* s=new string[10];
          for(int i=0;i<10;i++)
              s[i]= myMsg->getFlowData(i);
          DHTEntry entry = {
              s,
              simTime(),
              isInvalidData
            };
          saveFlowData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
      }

      if (myMsg->getType() == MYMSG_GET_FLOW) {
              //cout<<thisNode.getAddress()<<": GET_FLOW request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" NodeId:"<<myMsg->getOwnerNodeId()<<endl;
              DHTEntry * entry = const_cast < DHTEntry * > (retrieveFlowData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
              if(entry == NULL){
                  cout<<"MYMSG_GET_FLOW: DROPPING MESSAGE"<<endl;

                  delete myMsg;
                  return;
              }
              MyMessage *get_response_msg = new MyMessage();
              get_response_msg->setType(MYMSG_GET_FLOW_RESPONSE);
              for(int i=0;i<10;i++){
                  get_response_msg->setFlowData(i, entry->value[i].c_str());
              }

              get_response_msg->setIsInvalidData(entry->isInvalidData);
              get_response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
              get_response_msg->setSenderAddress(thisNode);
              sendMessageToUDP(myMsg->getSenderAddress(), get_response_msg);
          }

      if (myMsg->getType() == MYMSG_PUT_VOTE) {
                //cout<<"node"<<nodeId<<": PUT_FLOW request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;

                // if the responsible node is malicious and the target of the vote is the victim then the attacker can manipulate the data. (if integrity check is disabled)
                if(scenario5Attack && !isDataIntegrityEnabled && globalStatistics->isMalicious(nodeId) && globalStatistics->isVictim(myMsg->getTargetNodeId())){
                    globalStatistics->addControlledDataPair(nodeId, myMsg->getTargetNodeId(),"vote");
                }

              bool isInvalidData;
              double* s=new double();
              //The attacker corrupts flow data of the victim, with the aim of exclude it from the network.
                if(scenario5Attack && !isDataIntegrityEnabled && globalStatistics->isMalicious(nodeId) && globalStatistics->isVictim(myMsg->getTargetNodeId())){
                    isInvalidData=true;
                    *s = ANOMALOUS;
                    //TODO sostituire i flow normali con flow anomali.
                }
                else{
                    isInvalidData=false;
                    *s = myMsg->getVoteValue();
                }
                DHTDoubleEntry entry = {
                    s,
                    simTime(),
                    isInvalidData
                };
                saveVoteData(key,entry);
        }

      if (myMsg->getType() == MYMSG_GET_VOTE) {
             //       cout<<thisNode.getAddress()<<": GET_REPUTATION request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" OwnerNodeId:"<<myMsg->getOwnerNodeId()<<" TargetNodeId:"<<myMsg->getTargetNodeId()<<endl;
          //DHTDoubleEntry * entry = const_cast < DHTDoubleEntry * > (retrieveVoteData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
                    DHTDoubleEntry * entry = const_cast < DHTDoubleEntry * > (retrieveVoteData(key));
                    if(entry == NULL){
                        delete myMsg;
                        cout<<"MYMSG_GET_VOTE: DROPPING MESSAGE"<<endl;
                        return;
                    }
                    MyMessage *response_msg = new MyMessage();
                    response_msg->setType(MYMSG_GET_VOTE_RESPONSE);
                    response_msg->setVoteValue(entry->value[0]);
                    response_msg->setIsInvalidData(entry->isInvalidData);
                    response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
                    response_msg->setTargetNodeId(myMsg->getTargetNodeId());
                    response_msg->setSenderAddress(thisNode);
                   // if(globalStatistics->isMalicious(myMsg->getOwnerNodeId()) && globalStatistics->isVictim(myMsg->getTargetNodeId()))  //debug
                   //     cout<<"MYMSG_GET_VOTE: sending response "<<entry->value[0]<<" to "<<myMsg->getSenderAddress()<<" owner "<<myMsg->getOwnerNodeId()<<" target "<<myMsg->getTargetNodeId()<<" key is "<<myMsg->getKey().toString(16)<<endl;
                    sendMessageToUDP(myMsg->getSenderAddress(), response_msg);
                }
    delete myMsg;

}

// handleUDPMessage() is called when we receive a message from UDP.
void MyApplication::handleUDPMessage(cMessage* msg)
{
    // we are only expecting messages of type MyMessage
    MyMessage *myMsg = dynamic_cast<MyMessage*>(msg);
    if (myMsg && myMsg->getType() == MYMSG_GET_TOPOLOGY1_RESPONSE) {
        //cout<<thisNode.getAddress()<<": GET response from: "<<myMsg->getSenderAddress()<<endl;
        if(myMsg->getIsInvalidData())
            globalStatistics->invalidSensorDataGETNumber++;
        else
            globalStatistics->successfulSensorDataGETNumber++;
        //TODO Handle the TOPOLOGY1 sensor data
    }
    if (myMsg && myMsg->getType() == MYMSG_GET_TEMPERATURE_RESPONSE) {
            //cout<<thisNode.getAddress()<<": MYMSG_GET_TEMPERATURE_RESPONSE from: "<<myMsg->getSenderAddress()<<" isInvalidData: "<<myMsg->getIsInvalidData()<<endl;
        if(myMsg->getIsInvalidData())
            globalStatistics->invalidTemperatureDataGETNumber++;
        else
            globalStatistics->successfulTemperatureDataGETNumber++;
        //TODO Handle the received temperature data.
    }
    if (myMsg && myMsg->getType() == MYMSG_GET_MOTION_RESPONSE) {
        //cout<<thisNode.getAddress()<<": MYMSG_GET_MOTION_RESPONSE from: "<<myMsg->getSenderAddress()<<" isInvalidData: "<<myMsg->getIsInvalidData()<<endl;
            if(myMsg->getIsInvalidData())
                globalStatistics->invalidMotionDataGETNumber++;
            else
                globalStatistics->successfulMotionDataGETNumber++;
            //TODO Handle the received motion data.
    }
    if (myMsg && myMsg->getType() == MYMSG_GET_FLOW_RESPONSE) {
        //cout<<thisNode.getAddress()<<": MYMSG_GET_FLOW_RESPONSE from: "<<myMsg->getSenderAddress()<<" isInvalidData: "<<myMsg->getIsInvalidData()<<endl;
        for(int i=0;i<10;i++)
            flowDataArray[myMsg->getOwnerNodeId()][i] = myMsg->getFlowData(i);
    }

    if (myMsg && myMsg->getType() == MYMSG_GET_VOTE_RESPONSE) {
           // cout<<"node: "<<nodeId<<": MYMSG_GET_VOTE_RESPONSE OwnerNodeId: "<<myMsg->getOwnerNodeId()<<" TargetNodeId: "<<myMsg->getTargetNodeId()<<endl;
            int i= myMsg->getOwnerNodeId();
            int j= myMsg->getTargetNodeId();
            voteMatrix[i][j] = myMsg->getVoteValue();
    }
    // Message isn't needed any more -> delete it
    delete msg;
}


int MyApplication::generateNodeId(){
    nodes_counter++;
    return nodes_counter;
}

