

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


using namespace std;

#define N 500
#define flowPerSlot 10;
#define ANOMALOUS 1;
#define NORMAL 2;
#define NOT_AVAILABLE 3;

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
};/*
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
    //int flowPerSlot;
    bool node_is_started;
    int numSimNodes;
    int ttl; /**< ttl for stored DHT records */
    int numReplicas;
   // bool resourceExhaustionAttack;
    int scenario;
    bool isSybilEnabled;
  /*  bool invalidDataAttack;
    bool scenario2attack;
    bool scenario5attack;
    bool scenario6attack;
    bool isDataIntegrityEnabled;    */
    int maliciousLimit;
    int topology;
    int putTemperaturePeriod;
    int putMotionPeriod;
    double readTemperatureProbability;
    double readMotionProbability;
    bool *isInvalidClassificationDataArray;
    //If non-malicious node i performs an erroneous classification against another non-malicious node,
    //then the classification result can be used by the attacker for a replay attack.
    bool **isInvalidClassificationResultArray;
    // statistics
    int numSent; /**< number of sent packets*/
    int numGetSent; /**< number of get sent*/
    int numPutSent; /**< number of put sent*/
    int numReceived;          //number of packets received

    // our timer
    cMessage *timerMsg;

    // MediaSense statistics
    int simLookupTime;
    int numIterations;
    int nodeId;
    int simResetTime;

    //parameters for handling sensor data reading
    int sampleCounter;
    string * sensorData;
    char * sensorDataArray;
    //parameters for handling network data reading
    int classificationSlotNumber;
    string* classificationData;
    string** classificationDataArray;

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

/*
    //parameters for handling network data reading
    int flowCounter;
    Flow flow_array[N];*/
    //FlowContainer flowContainerArray[N];

    std::vector < OverlayKey > LUT_nodesKeys;  // data structure for storing all the nodes Kademlia key. (i'th element contains node i's key)

    //messages for timer events
    InfoMessage * openSHS_put_timer,
    *openSHS_get_timer,
    *UDPsend_timer,
    *openSHS_classification_timer,
    *put_temperature_timer,
    *get_temperature_timer,
    *put_motion_timer,
    *get_motion_timer,
    *put_classificationData_timer,
    *get_classificationData_timer,
    *put_classificationResult_timer,
    *get_vote_timer,
    * distributed_classification_timer;

    UnderlayConfigurator * underlayConfigurator; /**< pointer to UnderlayConfigurator in this node */
    GlobalNodeList * globalNodeList; /**< pointer to GlobalNodeList in this node*/

   // static const int DHTTESTAPP_VALUE_LEN = 20;

    void initializeApp(int stage);                 // called when the module is being created
    void finishApp();                              // called when the module is about to be destroyed
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

    void deleteApplicationNode();
    void updateTrust(int nodeId, int i, int finalResult);

    NodeHandle* getTransportAddress(std::string ip);
    void insertTransportAddress(std::string ip, NodeHandle& ta);
    int generateNodeId();
    //size_t size() { return dataMap.size(); };

    public:
      ~MyApplication() {
         cancelAndDelete(openSHS_put_timer);
         cancelAndDelete(openSHS_get_timer);
         cancelAndDelete(openSHS_classification_timer);
         cancelAndDelete(UDPsend_timer);
         cancelAndDelete(put_temperature_timer);
         cancelAndDelete(get_temperature_timer);
         cancelAndDelete(put_motion_timer);
         cancelAndDelete(get_motion_timer);

         LUT_nodesKeys.clear();
       }

       MyApplication() {

           openSHS_put_timer = NULL,
           openSHS_get_timer = NULL,
           UDPsend_timer = NULL,
           openSHS_classification_timer = NULL,
           put_temperature_timer = NULL,
           get_temperature_timer = NULL,
           put_motion_timer = NULL,
           get_motion_timer = NULL,
           put_classificationData_timer = NULL,
           get_classificationData_timer = NULL,
           put_classificationResult_timer = NULL,
           get_vote_timer = NULL,
           distributed_classification_timer = NULL;

           node_is_started = false;
       }
};
//static const int TEST_MAP_INTERVAL = 10; /**< interval in seconds for writing periodic statistical information */
static int nodes_counter = -1;


#endif
