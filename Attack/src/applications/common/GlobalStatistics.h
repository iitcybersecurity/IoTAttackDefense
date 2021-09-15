//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file GlobalStatistics.h
 * @author Ingmar Baumgart
 */

#ifndef __GLOBALSTATISTICS_H__
#define __GLOBALSTATISTICS_H__

#include <map>

#include <omnetpp.h>
#include <string>
#include "GlobalNodeList.h"

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
 * @author Ingmar Baumgart
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

    char** sensorDataArrayCollector;
    int sensorDataArrayCounter;
    bool isSensorDataArrayInvalid;

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

    bool** controlledSensorData;
    bool** controlledTemperatureData;
    bool** controlledMotionData;
    bool** controlledFlowData;
    bool ** controlledVoteData;

    int forward_counter0;
    int forward_counter1;
    int forward_counter2;
    //double maliciousNodesPercentage;
    int numMaliciousNodes;
    int targetNumMaliciousNodes;
    int numVictimNodes;
    int numTemperatureDevices;
    int numMotionDevices;
    bool* maliciousNodes;
    bool* victimNodes;
    int *temperatureDevices;
    int *motionDevices;
    int victimNodeId;
    int numDeliveredDataByMalicious;
    int numDeliveredDataByNonMalicious;
    int**voteMatrix;
    double addMaliciousNodePeriod;
    cMessage*addMaliciousNodeTimer;
    /*******/

    double sentKBRTestAppMessages; //!< total number of messages sent by KBRTestApp
    double deliveredKBRTestAppMessages; //!< total number of messages delivered by KBRTestApp
    int testCount;
    int numSimNodes;
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
    void setTemperatureAndMotionDevices(int,int);
    void setMaliciousNodes(int);
    void addControlledDataPair(int maliciousNode,int victimNode, std::string dataType);
    bool isMalicious(int);
    bool isTemperatureDevice(int);
    bool isMotionDevice(int);
    int getNumTemperatureDevices();
    int getNumMotionDevices();
    int* getTemperatureDevices();
    int* getMotionDevices();
    void notifyOrchestrator(int,int, int);
    void setVictimNodes(int);
    bool isVictim(int i);
    void addMaliciousNode();
    void removeVictimNode(int victimNodeId);
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
