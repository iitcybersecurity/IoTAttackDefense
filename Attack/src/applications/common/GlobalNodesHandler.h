
#ifndef __GLOBAL_NODES_HANDLER_H__
#define __GLOBAL_NODES_HANDLER_H__

#include <map>

#include <omnetpp.h>
#include <algorithm>
#include<iterator>
#include <vector>
#include <LifetimeChurn.h>
#include <UnderlayConfigurator.h>

#include <OverlayKey.h>
#include <BinaryValue.h>

#define N 500
#define flowPerSlot 10;
#define ANOMALOUS 1;
#define NORMAL 2;
#define NOT_AVAILABLE 3;

using namespace std;


class GlobalStatistics;



/**
 * Module with a global view on all currently stored DHT records (used
 * by DHTTestApp).
 *
 * @author Ingmar Baumgart
 */
class GlobalNodesHandler : public cSimpleModule
{
public:
    GlobalNodesHandler();
    ~GlobalNodesHandler();
    int scenario;
    int application;
    int numSimNodes;
    int numDevices;
    bool *isInvalidClassificationDataArray;
    //If non-malicious node i performs an erroneous classification against another non-malicious node,
    //then the classification result can be used by the attacker for a replay attack.
    bool **isInvalidClassificationResultArray;
    std::vector < OverlayKey > LUT_nodesKeys;  // data structure for storing all the nodes Kademlia key. (i'th element contains node i's key)
     //bool isSensorDataArrayInvalid;

    const OverlayKey& getRandomKey();
    uint32_t p2pnsNameCount;
    
    NodeHandle* getTransportAddress(std::string ip);
    void insertTransportAddress(std::string ip, NodeHandle& ta);
    int generateNodeId();
    void setTemperatureAndMotionDevices(int,int);
    void setMaliciousNodes(int);
    void setSybils(int);
    bool isMalicious(int);
    bool isTemperatureDevice(int);
    bool isMotionDevice(int);
    int getNumTemperatureDevices();
    int getNumMotionDevices();
    int getNumSybils();
    int* getTemperatureDevices();
    int* getMotionDevices();
    int* getSybils();
    void setVictimNodes(int);
    bool isVictim(int i);
    bool isSybil(int);
    void addMaliciousNode();
    void removeVictimNode(int victimNodeId);
    void setGroupA_Nodes(int groupA_nodesNumber);
    void setGroupB_Nodes(int groupB_nodesNumber);
    void addToGroupB();
    void removeGroupANode(int nodeId);
    bool isGroupANode(int nodeId);
    bool isGroupBNode(int nodeId);
    void deleteApplicationNode();
    void handleMessage(cMessage* msg);

    void initialize();

    UnderlayConfigurator* underlayConfigurator;
    simtime_t classificationSlotTime;
    int numMaliciousNodes;
    int targetNumMaliciousNodes;
    int numVictimNodes;
    int groupA_nodesNumber;
    int groupB_nodesNumber;
    int numTemperatureDevices;
    int numMotionDevices;
    int numSybils;
    bool* maliciousNodes;
    bool* victimNodes;
    bool* groupA_nodes;
    bool* groupB_nodes;
    int *temperatureDevices;
    int *motionDevices;
    int * sybilNodes;
    double addMaliciousNodePeriod;
    cMessage*addMaliciousNodeTimer;
    int nodes_counter;


    static const int TEST_MAP_INTERVAL = 10; 
    GlobalStatistics* globalStatistics; 
    cMessage *periodicTimer;
};

#endif
