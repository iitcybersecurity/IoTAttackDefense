
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
    //numSimNodes = par("numSimNodes");
    numSimNodes= par("targetOverlayTerminalNum");
    wrongClassificationProbability =par("wrongClassificationProbability");
    classificationSlotTime = par("classificationSlotTime");
    //flowPerSlot = par("flowPerSlot");
    topology=par("topology");
    putTemperaturePeriod = par("putTemperaturePeriod");
    putMotionPeriod = par("putMotionPeriod");
    readTemperatureProbability = par("readTemperatureProbability");
    readMotionProbability = par("readMotionProbability");
    scenario = par("scenario");
    isSybilEnabled = par("isSybilEnaled");
    trustThreshold = par("trustThreshold");
    reputationThreshold = par("reputationThreshold");
    beliefVar = par("beliefVar");
    disbeliefVar = par("disbeliefVar");
    reputationStrikesLimit = par("reputationStrikesLimit");
    //Attacks
    //resourceExhaustionAttack = par("resourceExhaustionAttack");
    //invalidDataAttack = par("invalidDataAttack");
    /*
    scenario2attack = par("scenario2attack");
    scenario5attack = par("scenario5attack");
    scenario6attack = par("scenario6attack");
    isDataIntegrityEnabled = par("isDataIntegrityEnabled");*/

    globalNodeList = GlobalNodeListAccess().get();
    underlayConfigurator = UnderlayConfiguratorAccess().get();
    globalStatistics = GlobalStatisticsAccess().get();

    //generate nodeId of this node
    nodeId = generateNodeId();
    numSent = 0;

    isInvalidClassificationDataArray=new bool[numSimNodes]();
    for(int i=0;i<numSimNodes;i++){
        isInvalidClassificationDataArray[i]=false;
    }
    isInvalidClassificationResultArray=new bool*[numSimNodes]();
    for(int i=0;i<numSimNodes;i++){
        isInvalidClassificationResultArray[i]=new bool[numSimNodes]();
        for(int j=0;j<numSimNodes;j++)
            isInvalidClassificationResultArray[i][j]=false;
    }
    //the array that will contain OPENSHS sensor data must be initialized
    sensorDataArray = new char[numSimNodes]();
     for(int i=0;i<numSimNodes;i++){
        sensorDataArray[i]='2'; //it means that the i-th node's sampled value hasn't been received yet
     }
     //Each node will read once all its sampled data from the dataset and put them periodically one by one into DHT.
     sensorData = new string[N];
     for(int i=0;i<N;i++){
         sensorData[i]="empty";
     }
     sampleCounter = 0; //It indicates the current index of the sampled data that must be put into DHT.

     //the array that will contain the entire classification data must be initialized
     classificationSlotNumber = -1;
     classificationData = new string[N];
     for(int i=0;i<N;i++){
         classificationData[i]="empty";
     }
     //the array that will contain the current classification data of all the other nodes must be initialized
     classificationDataArray = new string*[numSimNodes]();
     for(int i=0;i<numSimNodes;i++){
         classificationDataArray[i]=new std::string[10];
         for(int j=0;j<10;j++){
             classificationDataArray[i][j]="empty";
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
      //each entry of classification_results must be initializes with NOT_AVAILALE.
      classification_results = new double*[numSimNodes]();
        for(int i=0;i<numSimNodes;i++){
            classification_results[i]= new double[numSimNodes]();
            for(int j=0;j<numSimNodes;j++){
                classification_results[i][j]= NOT_AVAILABLE;
            }
        }

    //Create an entry for each node of the network, storing its overlay identifier
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
    openSHS_put_timer = new InfoMessage("openSHS_put_timer");
    openSHS_get_timer = new InfoMessage("openSHS_get_timer");
    put_temperature_timer = new InfoMessage("put_temperature_timer");
    get_temperature_timer = new InfoMessage("get_temperature_timer");
    put_motion_timer = new InfoMessage("put_motion_timer");
    get_motion_timer = new InfoMessage("get_motion_timer");
    put_classificationData_timer = new InfoMessage("put_classificationData_timer");
    get_classificationData_timer = new InfoMessage("get_classificationData_timer");
    openSHS_classification_timer = new InfoMessage("openSHS_classification_timer");
    put_classificationResult_timer = new InfoMessage("put_classificationResult_timer");
    get_vote_timer = new InfoMessage("get_vote_timer");
    distributed_classification_timer = new InfoMessage("distributed_classification_timer");
    UDPsend_timer = new InfoMessage("UDPsend_timer");


    fstream newfile;
    string str;
    //open the file to read my OPENSHS sensor data
    if(topology == 1){
        newfile.open("openSHS_dataset.txt", ios:: in );
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
    }

    //open the file to read network flow data
    if(topology==3){
        string s="network_dataset_" + thisNode.getIp().str()+ ".txt";
        newfile.open(s.c_str(), ios:: in );
        if (newfile.is_open()) {
              getline(newfile, str); //initial row must be discharged
              int line_counter=0;
              while (getline(newfile, str) && line_counter!=N) {
                  classificationData[line_counter] = str;
                  line_counter++;
              }
          }
        newfile.close();
    }

    if(topology == 1){
        //Put sampled data into dht, if available
        if(sensorData[0] != "empty")
            scheduleAt(simTime() + 15, openSHS_put_timer);
        scheduleAt(simTime() + 30, openSHS_get_timer);
        scheduleAt(simTime() + 31, openSHS_classification_timer);
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
    //classificationSlotTime is divided in 3 subslots. we assume that PUT/GET operations can be completed within classificationSlotTime/3 seconds
    if(topology == 3){
        if(classificationData[0] != "empty")
            scheduleAt(simTime() + classificationSlotTime, put_classificationData_timer);  //first at 100s
        scheduleAt(simTime() + classificationSlotTime + (classificationSlotTime/3), get_classificationData_timer); //first at 133s
        scheduleAt(simTime() + classificationSlotTime + (classificationSlotTime)*2/3, put_classificationResult_timer); //first at 166s
        //If attacks scenario is 6
        if(scenario == 61 || scenario == 62){
            scheduleAt(simTime() + classificationSlotTime*2 , get_vote_timer); //first at 200s
            scheduleAt(simTime() + classificationSlotTime*2 + classificationSlotTime/2, distributed_classification_timer); //first at 250s
        }
    }
    bindToPort(2000);
}

void MyApplication::deleteApplicationNode()
{
    LifetimeChurn* l = check_and_cast < LifetimeChurn * > (underlayConfigurator->getChurnGenerator(0));
    l->deleteNode(*(l->nodesList[20]),99);

    //TODO delete node from malicious nodes and victim nodes
}

void MyApplication::saveOpenSHSSensorData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    openSHSSensorDataMap.erase(key);
    openSHSSensorDataMap.insert(make_pair(key, entry));
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

void MyApplication::saveClassificationData(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    classificationDataMap.erase(key);
    classificationDataMap.insert(make_pair(key, entry));


}

void MyApplication::saveVoteData(const OverlayKey& key, const DHTDoubleEntry& entry)
{
    Enter_Method_Silent();

    voteDataMap.erase(key);
    voteDataMap.insert(make_pair(key, entry));
}


const DHTEntry* MyApplication::retrieveOpenSHSSensorData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = openSHSSensorDataMap.find(key);

    if (it == openSHSSensorDataMap.end()) {
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

const DHTEntry* MyApplication::retrieveClassificationData(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = classificationDataMap.find(key);

    if (it == classificationDataMap.end()) {
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
  if (myMsg -> isName("openSHS_put_timer")) {
    try {

      MyMessage *myMessage; // the message we'll send
      myMessage = new MyMessage();
      myMessage->setType(MYMSG_PUT_OPENSHS_DATA); // set the message type
      myMessage->setTimeSlot(sampleCounter);
      std::stringstream ss;
      ss << "node" << nodeId <<"_sensorData";
      std::string node = ss.str();
      BinaryValue binValue(node);
      myMessage->setKey(OverlayKey::sha1(binValue)); //Set the key of the data
      myMessage->setOwnerNodeId(nodeId); //Set the id of the node that originated the data
      //If a scenario 2 attack is performed the attacker must put fictitious sampled data into dht.
      if(scenario == 2 && globalStatistics->isMalicious(nodeId)){
          myMessage->setDetectedValue("null");
          myMessage->setIsInvalidData(true);
      }
      else
          myMessage->setDetectedValue(sensorData[sampleCounter].c_str());
      myMessage->setSenderAddress(thisNode);
      callRoute(myMessage->getKey(), myMessage);
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
  else if (myMsg -> isName("openSHS_get_timer")) {
    for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
          MyMessage *myMessage;
          myMessage = new MyMessage();
          myMessage->setType(MYMSG_GET_OPENSHS_DATA); // set the message type
          std::stringstream ss;
          ss << "node" << i <<"_sensorData";  //debug
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
            //If attack scenario 2 is performed
            if(scenario == 2 && globalStatistics->isMalicious(nodeId))
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
              //If scenario 2 attack is performed
              if(scenario == 2 && globalStatistics->isMalicious(nodeId))
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
        scheduleAt(simTime() + putMotionPeriod, put_motion_timer);
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


  if (myMsg -> isName("openSHS_classification_timer")) {
      //TODO classification of sensorDataArray

      scheduleAt(simTime() + 3, myMsg);
  }

  if (myMsg -> isName("put_classificationData_timer")) {
      try {
          classificationSlotNumber++;
          for(int i=0;i<numSimNodes;i++){  //One replica of classification data for each other node
                if(i == nodeId)
                    continue;
                MyMessage *myMessage;
                myMessage = new MyMessage();
                myMessage->setType(MYMSG_PUT_CLASSIFICATION_DATA);
                std::stringstream ss;
                ss << "node" << nodeId <<"_classification_data_replica"<<i;
                std::string node = ss.str();
                BinaryValue binValue(node);
                myMessage->setKey(OverlayKey::sha1(binValue));
                myMessage->setOwnerNodeId(nodeId);
                int index=classificationSlotNumber*10;     //We consider 10 flows inside the flow classification data for each slot time
                for(int j=0;j<10;j++){
                    myMessage->setClassificationData(j, classificationData[index + j].c_str());
                }
                myMessage->setSenderAddress(thisNode);
                callRoute(myMessage->getKey(), myMessage);
          }

            int index = (classificationSlotNumber+1)*10;
            if(classificationData[index] == "empty"){  //if next slot's flow data is not available then i stop
                 return;
            }
            scheduleAt(simTime() + classificationSlotTime, myMsg);
      }
      catch (...) {
            cout << "\n Insertion failed.";
            return;
          }
    }

  if (myMsg -> isName("get_classificationData_timer")){
      for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
                if(i==nodeId)
                    continue;
                MyMessage *myMessage;
                myMessage = new MyMessage();
                myMessage->setType(MYMSG_GET_CLASSIFICATION_DATA); // set the message type

                std::stringstream ss;
                //I get only my own copy of node i's flow data
                ss << "node" << i <<"_classification_data_replica"<<nodeId;
                std::string node = ss.str();
                BinaryValue binValue(node);
                myMessage->setKey(OverlayKey::sha1(binValue));
                myMessage->setOwnerNodeId(i);
                myMessage->setRequesterNodeId(nodeId);
                myMessage->setSenderAddress(thisNode);
                callRoute(OverlayKey::sha1(binValue), myMessage);
       }
       scheduleAt(simTime() + classificationSlotTime, myMsg);
   }
  if (myMsg -> isName("put_classificationResult_timer")) {

          for(int i=0;i<numSimNodes;i++){
              if(i!=nodeId){
                  if(globalStatistics->isMalicious(nodeId)){
                      if(globalStatistics->isVictim(i)){
                          //cout<<nodeId<<" voting against "<<i<<endl;
                          classification_results[nodeId][i]= ANOMALOUS ;
                      }
                      else{
                          classification_results[nodeId][i]= NORMAL ;
                      }
                   }
                   else{
                          //TODO CLASSIFICAZIONE DI classificationDataArray[i][j]
                          //if im not a malicious node and DATA FLOW HAVE BEEN MODIFIED
                       int anomalous= ANOMALOUS;
                       int normal= NORMAL;
                       int not_available = NOT_AVAILABLE;
                       //If classification data is invalid and
                       if(isInvalidClassificationDataArray[i]){
                           if(scenario == 51)
                               classification_results[nodeId][i]= not_available;
                           if(scenario == 52)
                               classification_results[nodeId][i]= anomalous;
                       }
                       //IF data have not been modified i proceed with normal classification
                       else{

                           srand(time(NULL));
                           int outcome=rand()%100;
                           if(outcome<wrongClassificationProbability){
                               classification_results[nodeId][i]= anomalous;    //Erroneous classificaiton
                               isInvalidClassificationResultArray[nodeId][i]=true;
                           }
                           else
                               classification_results[nodeId][i]= normal;
                       }
                   }
                  //If scenario5 then we send the vote to the orchestator, else we publish it into DHT.
                   if(scenario == 51 || scenario == 52){
                       globalStatistics->notifyOrchestrator(nodeId,i, classification_results[nodeId][i]);
                   }
                   if(scenario == 61 || scenario == 62){
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
                       myMessage->setVoteValue(classification_results[nodeId][i]);
                       myMessage->setSenderAddress(thisNode);
                       callRoute(myMessage->getKey(), myMessage);
                   }
              }
          }

     scheduleAt(simTime() + classificationSlotTime, myMsg);
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
        scheduleAt(simTime() + classificationSlotTime, myMsg);
     }

  if (myMsg -> isName("distributed_classification_timer")){
         // cout << "Node: " << nodeId << ", distributed_classification_timer."<< endl;

          if(!globalStatistics->isMalicious(nodeId)){
              //for (unsigned int i = 0; i < globalNodeList -> getNumNodes(); ++i) {
                  int nodeToClassify=(nodeId+classificationSlotNumber)%numSimNodes;
                  if(nodeToClassify==nodeId)
                      nodeToClassify=(nodeToClassify+1)%numSimNodes;

                  double anomalousValidity = 0;
                  double normalValidity = 0;
                  double k= ANOMALOUS;
                  if(classification_results[nodeId][nodeToClassify] == k){
                      anomalousValidity = anomalousValidity +1;
                  }
                  else{
                      normalValidity = normalValidity+1;
                  }

                  for (unsigned int j = 0; j < globalNodeList -> getNumNodes(); ++j) {
                      if(nodeToClassify!=nodeId && j!= nodeId && nodeToClassify!=j){
                          if(trust[j][0]-trust[j][1] >= trustThreshold){  //belief - disbelief > threshold?
                              int anomalous = ANOMALOUS;
                              double normal = NORMAL;
                              double not_available= NOT_AVAILABLE;
                              if(classification_results[j][nodeToClassify] == anomalous)
                                  anomalousValidity = anomalousValidity + trust[j][0]-trust[j][1];
                              if(classification_results[j][nodeToClassify] == normal)
                                  normalValidity = normalValidity + trust[j][0]-trust[j][1];
                              if(classification_results[j][nodeToClassify] == not_available && nodeId == 0){  //debug
                                  cout<<"node "<<nodeId<<"didn't get vote of "<<j<<" to "<<nodeToClassify<<endl;
                              }
                              if(classification_results[j][nodeToClassify] != not_available && classification_results[j][nodeToClassify] != anomalous && classification_results[j][nodeToClassify] != normal && nodeId == 0){  //debug
                                cout<<"invalid vote: "<<j<<" to "<<nodeToClassify<<" vote is "<<classification_results[j][nodeToClassify]<<endl;

                              }
                          }

                      }
                  }
                  reputation[nodeToClassify][0] = normalValidity;
                  reputation[nodeToClassify][1] = anomalousValidity;

                  double anomalous= ANOMALOUS;
                  double normal = NORMAL;
                  int finalResult = ((reputation[nodeToClassify][0]/(reputation[nodeToClassify][0]+reputation[nodeToClassify][1])) >= reputationThreshold)?normal:anomalous;

                  //After we classify node i as normal or anomalous we update the trust of the nodes before proceeding with the next iteration
                  updateTrust(nodeId, nodeToClassify, finalResult);

                  if(finalResult == anomalous){
                      reputation[nodeToClassify][2]++;
                      if(reputation[nodeToClassify][2]==reputationStrikesLimit){
                          cout<<"NODE "<<nodeId<<"classified NODE"<<nodeToClassify<<"as malicious!"<<endl;
                          endSimulation();
                      }
                  }
               /*   if(nodeId==0){ //debug

                      cout<<"node"<<i<<" reputation: "<<reputation[i][0]<<" , "<<reputation[i][1]<<"   mean:"<<reputation[i][0]/(reputation[i][0]+reputation[i][1])<<" ";
                      cout<<" updated trust :";
                      for(int j=0;j<numSimNodes;j++)
                              cout<<"  node"<<j<<":("<<trust[j][0]<<","<<trust[j][1]<<"),";
                      cout<<endl;
                  }*/
              //}
              for(int i=0;i<numSimNodes;i++)  //RESET VOTES
                for(int j=0;j<numSimNodes;j++)
                  classification_results[i][j] = NOT_AVAILABLE;

          }
        scheduleAt(simTime() + classificationSlotTime, myMsg);
  }

}


// finish is called when the module is being destroyed
void MyApplication::finishApp(){
    // finish() is usually used to save the module's statistics.
    // We'll use globalStatistics->addStdDev(), which will calculate min, max, mean and deviation values.
    // The first parameter is a name for the value, you can use any name you like (use a name you can find quickly!).
    // In the end, the simulator will mix together all values, from all nodes, with the same name.

    //TODO register statistics
    globalStatistics->addStdDev("MyApplication: Sent packets", numSent);
}

void MyApplication::updateTrust(int nodeId, int i, int finalResult){
    for(int j=0;j<numSimNodes;j++){
        if(nodeId!=j && nodeId!= i && i!=j){
            if(finalResult == classification_results[j][i]){
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
            globalStatistics -> registerTrust(j,trust[j][0]);
        }
  }
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

    if (myMsg->getType() == MYMSG_PUT_OPENSHS_DATA) {
        // if a scenario 2 attack is performed the adversary can control all the data that he must store in DHT.
           if(scenario == 2 && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"openshs");
                myMsg->setDetectedValue("null");
                myMsg->setIsInvalidData(true);

           }
           bool isInvalidData;
           if(scenario == 2 && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
               isInvalidData=true;
           }
           else
               isInvalidData=false;
           string* s=new string[1]();
           s[0]=myMsg->getDetectedValue();
           DHTEntry entry = {
                s,
                simTime(),
                myMsg->getIsInvalidData()
              };
            saveOpenSHSSensorData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
     }

    if (myMsg->getType() == MYMSG_GET_OPENSHS_DATA) {
        DHTEntry * entry = const_cast < DHTEntry * > (retrieveOpenSHSSensorData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
        if(entry == NULL){
            delete myMsg;
            cout<<"MYMSG_GET_OPENSHS: DROPPING MESSAGE"<<endl;
            return;
        }
        MyMessage *response_msg = new MyMessage();
        response_msg->setType(MYMSG_GET_OPENSHS_RESPONSE);
        response_msg->setDetectedValue(entry->value[0].c_str());
        response_msg->setIsInvalidData(entry->isInvalidData);
        response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
        response_msg->setSenderAddress(thisNode);
        sendMessageToUDP(myMsg->getSenderAddress(), response_msg);
    }

    if (myMsg->getType() == MYMSG_PUT_TEMPERATURE) {
    //        cout<<"node"<<nodeId<<": PUT_TEMPERATURE request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;
        // if the responsible node is malicious and the owner of the temperature data is the victim then the attacker can manipulate the temperature data. (if integrity check is disabled)
            if(scenario == 2 && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"temperature");
            }
            bool isInvalidData;
            if(scenario == 2 && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
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
                if(scenario == 2 && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
                    globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"motion");
                }
                bool isInvalidData;
                if(scenario == 2 && (globalStatistics->isMalicious(nodeId) || globalStatistics->isMalicious(myMsg->getOwnerNodeId()))){
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
      if (myMsg->getType() == MYMSG_PUT_CLASSIFICATION_DATA) {

     /*     if((scenario5attack || scenario6attack)  && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
              globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"flow");
          }*/
          bool isInvalidData;
          // if the responsible node is malicious and the owner of the CLASSIFICATION data is the victim then the attacker can manipulate the classification data.
          //In case of scenario 5.1 the classification data must not be returned
          //In case of scenario 5.2 the classification data must be replaced with an old authenticated data that describes an anomalous behavior
          if((scenario == 51 || scenario == 52) && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId())){
              globalStatistics->addControlledDataPair(nodeId, myMsg->getOwnerNodeId(),"classification_data");
              isInvalidData=true;
          }
          else
              isInvalidData=false;
          string* s=new string[10];
          for(int i=0;i<10;i++)
              s[i]= myMsg->getClassificationData(i);
          DHTEntry entry = {
              s,
              simTime(),
              isInvalidData
          };
          saveClassificationData(LUT_nodesKeys[myMsg->getOwnerNodeId()],entry);
      }

      if (myMsg->getType() == MYMSG_GET_CLASSIFICATION_DATA) {
              //cout<<thisNode.getAddress()<<": GET_FLOW request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" NodeId:"<<myMsg->getOwnerNodeId()<<endl;
              DHTEntry * entry = const_cast < DHTEntry * > (retrieveClassificationData(LUT_nodesKeys[myMsg->getOwnerNodeId()]));
              if(entry == NULL){
                  cout<<"MYMSG_GET_CLASSIFICATION_DATA: DROPPING MESSAGE"<<endl;

                  delete myMsg;
                  return;
              }
              MyMessage *get_response_msg = new MyMessage();
              get_response_msg->setType(MYMSG_GET_CLASSIFICATION_DATA_RESPONSE);
              for(int i=0;i<10;i++){
                  get_response_msg->setClassificationData(i, entry->value[i].c_str());
              }

              get_response_msg->setIsInvalidData(entry->isInvalidData);
              get_response_msg->setOwnerNodeId(myMsg->getOwnerNodeId());
              get_response_msg->setSenderAddress(thisNode);
              sendMessageToUDP(myMsg->getSenderAddress(), get_response_msg);
          }

      if (myMsg->getType() == MYMSG_PUT_VOTE) {
                //cout<<"node"<<nodeId<<": PUT_VOTE request from node"<<myMsg->getOwnerNodeId()<<". key: "<<key.toString(16)<<endl;


              bool isInvalidData;
              double* s=new double();
              //in case of scenario6.2, the attacker manipulates the votes against the victims, with the aim of excluding it from the network.
              //If the node had previously performed an erroneous classification against another node then the classification result can be reused.
                if((scenario == 62) && globalStatistics->isMalicious(nodeId) && !globalStatistics->isMalicious(myMsg->getOwnerNodeId()) && globalStatistics->isVictim(myMsg->getTargetNodeId()) && isInvalidClassificationResultArray[myMsg->getOwnerNodeId()][myMsg->getTargetNodeId()]){
                    globalStatistics->addControlledDataPair(nodeId, myMsg->getTargetNodeId(),"classification_result");
                    isInvalidData=true;
                    *s = ANOMALOUS;
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
             //       cout<<thisNode.getAddress()<<": GET_VOTE request from: "<<myMsg->getRequesterNodeId()<<". key: "<<key.toString(16)<<" OwnerNodeId:"<<myMsg->getOwnerNodeId()<<" TargetNodeId:"<<myMsg->getTargetNodeId()<<endl;
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
    if (myMsg && myMsg->getType() == MYMSG_GET_OPENSHS_RESPONSE) {
        //cout<<thisNode.getAddress()<<": GET response from: "<<myMsg->getSenderAddress()<<endl;
        if(myMsg->getIsInvalidData())
            globalStatistics->invalidSensorDataGETNumber++;
        else
            globalStatistics->successfulSensorDataGETNumber++;
        //TODO Handle the OPENSHS sensor data
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
    if (myMsg && myMsg->getType() == MYMSG_GET_CLASSIFICATION_DATA_RESPONSE) {
        //cout<<thisNode.getAddress()<<": MYMSG_GET_CLASSIFICAITON_DATA_RESPONSE from: "<<myMsg->getSenderAddress()<<" isInvalidData: "<<myMsg->getIsInvalidData()<<endl;

        for(int i=0;i<10;i++)
            classificationDataArray[myMsg->getOwnerNodeId()][i] = myMsg->getClassificationData(i);
        if(myMsg->getIsInvalidData())//DEBUG
            isInvalidClassificationDataArray[myMsg->getOwnerNodeId()]=true;
        else
            isInvalidClassificationDataArray[myMsg->getOwnerNodeId()]=false;
    }

    if (myMsg && myMsg->getType() == MYMSG_GET_VOTE_RESPONSE) {
           // cout<<"node: "<<nodeId<<": MYMSG_GET_VOTE_RESPONSE OwnerNodeId: "<<myMsg->getOwnerNodeId()<<" TargetNodeId: "<<myMsg->getTargetNodeId()<<endl;
            int i= myMsg->getOwnerNodeId();
            int j= myMsg->getTargetNodeId();
            classification_results[i][j] = myMsg->getVoteValue();
    }
    // Message isn't needed any more -> delete it
    delete msg;
}


int MyApplication::generateNodeId(){
    nodes_counter++;
    return nodes_counter;
}


