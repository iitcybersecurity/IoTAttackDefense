#ifndef __GLOBALSTATISTICS_H__
#define __GLOBALSTATISTICS_H__

#include <map>

#include <omnetpp.h>
#include <string>
#include "GlobalNodeList.h"
#include "MyApplication.h"
#include "GlobalNodesHandler.h"

using namespace std;
class OverlayKey;

/**
 * Macro used for recording statistics considering measureNetwIn
 * parameter. The do-while-loop is needed for comparability in
 * outer if-else-structures.
 */
#define RECORD_STATS(x) \
    do { \
        if (globalStatistics->isMeasuring()){ x; } \
    } while(false)



/**
 * Module to record global statistics
 *
 */

struct MeanStatistic{
    int counter;
    double accumulator;
};

class GlobalStatistics : public cSimpleModule
{
public:
    static const double MIN_MEASURED; //!< minimum useful measured lifetime in seconds

    /********/
    UnderlayConfigurator* underlayConfigurator;
    //GlobalNodesHandler* globalNodesHandler;
    int scenario;
    int numDevices;
    int numSimNodes;
    simtime_t classificationSlotTime;
    int numMaliciousNodes;

    MeanStatistic invalidSensorDataPercentageMean;
    int numSensorDataConsistencies;
    int numSensorDataInconsistencies;
    int numInvalidSensorDataArrays;

    int invalidSensorDataGETNumber;
    int invalidTemperatureDataGETNumber;
    int invalidMotionDataGETNumber;
    int successfulSensorDataGETNumber;
    int successfulTemperatureDataGETNumber;
    int successfulMotionDataGETNumber;

    

    //Variables for evaluating mean trust
    int maximumClassificationCycles;
    double *averageMaliciousTrustCollector;
    double averageMaliciousTrustAccumulator;
    double averageMaliciousTrustCounter;
    double *averageNormalTrustCollector;
    double averageNormalTrustAccumulator;
    double averageNormalTrustCounter;
    int slotNumber;
    cOutVector averageMaliciousTrustVector;
    cOutVector averageNormalTrustVector;

    bool** controlledSensorData;
    bool** controlledTemperatureData;
    bool** controlledMotionData;
    bool** controlledClassificationData;
    bool ** controlledVoteData;
    int**voteMatrix;
    char** sensorDataArrayCollector; //data structure that collects all the OPENSHS sensor data array of each node.
    int sensorDataArrayCounter; //Index of the last inserted sensor data array

    double sentKBRTestAppMessages; //!< total number of messages sent by KBRTestApp
    double deliveredKBRTestAppMessages; //!< total number of messages delivered by KBRTestApp
    int testCount;
    cOutVector currentDeliveryVector; //!< statistical output vector for current delivery ratio

    /**
     * Destructor
     */
    ~GlobalStatistics();

    /**
     * Add a new value to the cStdDev container specified by the name parameter.
     * If the container does not exist yet, a new container is created
     *
     * @param name a string to identify the container (should be "Module: Scalar Name")
     * @param value the value to add
     */
    void addStdDev(const std::string& name, double value);

    /**
     * Add a value to the histogram plot, or create a new histogram if one
     * hasn't yet been created with name.
     */
    void recordHistogram(const std::string& name, double value);

    /**
     * Record a value to a global cOutVector defined by name
     *
     * @param name a string to identify the vector (should be "Module: Scalar Name")
     * @param value the value to add
     */
    void recordOutVector(const std::string& name, double value);

    void startMeasuring();

    inline bool isMeasuring() { return measuring; };
    inline bool getMeasureNetwInitPhase() { return measureNetwInitPhase; };
    inline simtime_t getMeasureStartTime() { return measureStartTime; };

    simtime_t calcMeasuredLifetime(simtime_t creationTime);

    void finalizeStatistics();

    /*****/
    void compareSensorDataArrays(char* sensorDataArray);
    void addControlledDataPair(int maliciousNode,int victimNode, std::string dataType);
    
    void handleTimerEvent(cMessage* msg);          // called when we received a timer message
    void registerTrust(int nodeId,int trust,bool);
    void notifyOrchestrator(int,int, int);

    /****/
protected:

    struct OutVector //!< struct for cOutVectors and cummulated values
    {
        cOutVector vector; //!< output vector
        int count; //!< number of recorded values
        double value; //!< sum of values
        double avg;

        OutVector(const std::string& name) :
            vector(name.c_str()), count(0), value(0), avg(0) {};
    };

    std::map<std::string, cStdDev*> stdDevMap; //!< map to store and access scalars
    std::map<std::string, cHistogram*> histogramMap; //!< map to store and access histograms
    std::map<std::string, OutVector*> outVectorMap; //!< map to store and access the output vectors
    cMessage* globalStatTimer; //!< timer for periodic statistic updates
    double globalStatTimerInterval; //!< interval length of periodic statistic timer

    /**
     * Init member function of module
     */
    virtual void initialize();

    /**
     * HandleMessage member function of module
     */
    virtual void handleMessage(cMessage* msg);

    /**
     * Finish member function of module
     */
    virtual void finish();

    bool measuring;
    bool measureNetwInitPhase;
    simtime_t measureStartTime;
};

#endif
