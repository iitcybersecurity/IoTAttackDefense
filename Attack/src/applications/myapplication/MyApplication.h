

#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H


#include <omnetpp.h>

#include <GlobalNodeList.h>
#include <GlobalStatistics.h>
#include <UnderlayConfigurator.h>
#include <TransportAddress.h>
#include <OverlayKey.h>
#include <InitStages.h>
#include <BinaryValue.h>
#include <BaseApp.h>
#include <set>
#include <sstream>
#include <InfoMessage_m.h>
#include <MyMessage_m.h>
#include<string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <GlobalNodesHandler.h>

#include <IPAddressResolver.h>
#include <TransportAddress.h>
#include <GlobalNodeListAccess.h>
#include <GlobalStatisticsAccess.h>
#include <GlobalNodesHandlerAccess.h>
#include <UnderlayConfiguratorAccess.h>
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
#include "GlobalStatistics.h"
#include "MyMessage_m.h"
#include "GlobalNodesHandler.h"


using namespace std;

struct RNG {
    int operator() (int n) {
        return std::rand() / (1.0 + RAND_MAX) * n;
    }
};

struct DHTDoubleEntry
{
    double* value;
    simtime_t insertiontime;
    bool isInvalidData;
    friend std::ostream& operator<<(std::ostream& Stream, const DHTDoubleEntry entry);
};

struct DHTEntry
{
    string* value;
    simtime_t insertiontime;
    bool isInvalidData;
    friend std::ostream& operator<<(std::ostream& Stream, const DHTEntry entry);
};
/*
struct Flow{
    bool init;

    std::string src_ip;
    std::string dest_ip;
    std::string timestamps;
    std::string flow_duration;
    std::string total_fwd_packets;
    std::string total_bwd_packets;
    std::string total_fwd_len;
    std::string total_bwd_len;
    std::string fwd_packets_len_max;
    std::string fwd_packets_len_min;
    std::string fwd_packets_len_mean;
    std::string  fwd_packets_len_std;
    std::string  bwd_packets_len_max;
    std::string  bwd_packets_len_min;
    std::string bwd_packets_len_mean;
    std::string bwd_packets_len_std;
    std::string fwd_IAT_mean;
    std::string fwd_IAT_std;
    std::string fwd_IAT_max;
    std::string fwd_IAT_min;
    std::string bwd_IAT_mean;
    std::string bwd_IAT_std;
    std::string bwd_IAT_max;
    std::string bwd_IAT_min;
    std::string fwd_PSH_flags;
    std::string bwd_PSH_flags;
    std::string fwd_URG_flags;
    std::string bwd_URG_flags;
    std::string fwd_header_len;
    std::string bwd_header_len;
    std::string subflow_fwd_packets;
    std::string subflow_fwd_bytes;
    std::string subflow_bwd_packets;
    std::string subflow_bwd_bytes;
    std::string active_mean;
    std::string active_std;
    std::string active_max;
    std::string active_min;
    std::string idle_mean;
    std::string idle_std;
    std::string idle_max;
    std::string idle_min;
    std::string slot;
};
struct FlowContainer{
    vector<Flow> flowVector;
    int sizee; //debug
};*/
class MyApplication : public BaseApp
{
    private:
        class DHTStatsContext: public cPolymorphic {
              public: bool measurementPhase;
              simtime_t requestTime;
              OverlayKey key;
              BinaryValue value;

              DHTStatsContext(bool measurementPhase, simtime_t requestTime,
                const OverlayKey & key,
                  const BinaryValue & value = BinaryValue::UNSPECIFIED_VALUE): measurementPhase(measurementPhase),
              requestTime(requestTime),
              key(key),
              value(value) {};
        };

    // module parameters
    simtime_t classificationSlotTime;
    bool node_is_started;
    int ttl; /**< ttl for stored DHT records */
    int numReplicas;
    bool isSybilEnabled;
    int putTemperaturePeriod;
    int putMotionPeriod;
    double readTemperatureProbability;
    double readMotionProbability;
    int nodeId;
    int numSimNodes;
    int numDevices;

    //Parameters for handling sensor data reading
    int sampleCounter;
    string * sensorData;
    char * sensorDataArray;
    //Parameters for handling network data reading
    int classificationSlotNumber;
    string* classificationData;
    string** classificationDataArray;

    //Parameters for Intrusion Detection System
    double** reputation;
    double** trust;
    double** classification_results;
    double trustThreshold;
    double reputationThreshold;
    double beliefVar;
    double disbeliefVar;
    int reputationStrikesLimit;
    double wrongClassificationProbability;


    std::map<OverlayKey, DHTEntry> openSHSSensorDataMap;
    std::map<OverlayKey, DHTEntry> temperatureDataMap;
    std::map<OverlayKey, DHTEntry> motionDataMap;
    std::map<OverlayKey, DHTEntry> classificationDataMap;
    std::map<OverlayKey, DHTDoubleEntry> reputationDataMap;
    std::map<OverlayKey, DHTDoubleEntry> voteDataMap;
    

    //messages for timer events
    InfoMessage * openSHS_put_timer, //for handling PUT operation of openSHS sensor data
    *openSHS_get_timer,	//for handling GET operation of openSHS sensor data
    *openSHS_classification_timer, //for handling classification of user's activity
    *put_temperature_timer,	////for handling PUT operation of temperature data in application 2 
    *get_temperature_timer, ////for handling GET operation of temperature data in application 2 
    *put_motion_timer, ////for handling PUT operation of motion data in application 2 
    *get_motion_timer, ////for handling GET operation of MOTION data in application 2 
    *put_classificationData_timer, ////for handling PUT operation of classificationd data in application3
    *get_classificationData_timer, ////for handling GET operation of classification data in application 3 
    *put_classificationResult_timer,  ////for handling put operation of classification results in application 3
    *get_classificationResult_timer, ////for handling get operation of classification results in application 3
    * distributed_classification_timer;  //for handling distributed classification according to josang trust model

    UnderlayConfigurator * underlayConfigurator; /**< pointer to UnderlayConfigurator in this node */
    GlobalNodeList * globalNodeList; /**< pointer to GlobalNodeList in this node*/
    GlobalNodesHandler* globalNodesHandler;  /**< pointer to GlobalNodesHandler in this node*/


    void initializeApp(int stage);                 // called when the module is being created
    //void finishApp();                              // called when the module is about to be destroyed
    void handleTimerEvent(cMessage* msg);          // called when we received a timer message
    void deliver(OverlayKey& key, cMessage* msg);  // called when we receive a message from the overlay
    void handleUDPMessage(cMessage* msg);          // called when we receive a UDP message

    void saveOpenSHSSensorData(const OverlayKey& key, const DHTEntry& entry);
    void saveTemperatureData(const OverlayKey& key, const DHTEntry& entry);
    void saveMotionData(const OverlayKey& key, const DHTEntry& entry);
    void saveClassificationData(const OverlayKey& key, const DHTEntry& entry);
    void saveReputationData(const OverlayKey& key, const DHTDoubleEntry& entry);
    void saveVoteData(const OverlayKey& key, const DHTDoubleEntry& entry);

    const DHTEntry* retrieveOpenSHSSensorData(const OverlayKey& key);
    const DHTEntry* retrieveTemperatureData(const OverlayKey& key);
    const DHTEntry* retrieveMotionData(const OverlayKey& key);
    const DHTEntry* retrieveClassificationData(const OverlayKey& key);
    const DHTDoubleEntry* retrieveReputationData(const OverlayKey& key);
    const DHTDoubleEntry* retrieveVoteData(const OverlayKey& key);

    void updateTrust(int nodeId, int i, int finalResult);

    //size_t size() { return dataMap.size(); };

    public:
      ~MyApplication() {
         cancelAndDelete(openSHS_put_timer);
         cancelAndDelete(openSHS_get_timer);
         cancelAndDelete(openSHS_classification_timer);
         cancelAndDelete(put_temperature_timer);
         cancelAndDelete(get_temperature_timer);
         cancelAndDelete(put_motion_timer);
         cancelAndDelete(get_motion_timer);

         cancelAndDelete(put_classificationData_timer);         
         cancelAndDelete(get_classificationData_timer);
         cancelAndDelete(put_classificationResult_timer);
         cancelAndDelete(get_classificationResult_timer);
         cancelAndDelete(distributed_classification_timer);
       }

       MyApplication() {

           openSHS_put_timer = NULL,
           openSHS_get_timer = NULL,
           openSHS_classification_timer = NULL,
           put_temperature_timer = NULL,
           get_temperature_timer = NULL,
           put_motion_timer = NULL,
           get_motion_timer = NULL,
           put_classificationData_timer = NULL,
           get_classificationData_timer = NULL,
           put_classificationResult_timer = NULL,
           get_classificationResult_timer = NULL,
           distributed_classification_timer = NULL;

           node_is_started = false;
       }
};


#endif
