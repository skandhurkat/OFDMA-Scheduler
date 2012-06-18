//user.h
//© Skand Hurkat, 2012

/*     This file is part of OFDMA Scheduler.
 *
 *     OFDMA Scheduler is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     OFDMA Scheduler is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with OFDMA Scheduler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USER_H_INCLUDED
#define USER_H_INCLUDED

#include <queue>
#include <inttypes.h>
#include "trafficModel.h"
#include "packet.h"
#include "coordinates.h"

using namespace std;

class user
{
private:
    static const uint8_t NUM_CHANNELS = 14;     //20MHz/1.4MHz per user means 14 logical channels available for every user.
    static const float MIN_DIST = 35;
    static const float MAX_DIST = 300;
    static const float CELL_RADIUS = 500;
    static const float TRANSMIT_POWER = 40;
    static const float THERMAL_NOISE = 8E-14;
    static const uint32_t UPDATE_SHADOWING_AFTER = 100;
    static const float SHADOWING_SIGMA = 0.89;
    static const float MEAN_RAYLEIGH_FADING = 0.4698;
    static const float DIST_ATTENUATING_FACTOR = 3.5;
    static const uint16_t CQI2DataRate[16];
    static const float beta[16];
    static const float SINRthresh[16];

    bool initialized;

    coordinates position;
    uint32_t* volatile simulationTime;
    uint32_t timeLastUpdated;
    uint32_t timeShadowingLastUpdated;
    float shadowingParams[7];
    priority_queue<packet> dataQueue;
    uint32_t bitsInQueue;
    trafficModel userTrafficModel;
    bool** volatile channelStateForNeighbours;
    uint16_t dataRate[NUM_CHANNELS];

    void placeUser();
    void generateTraffic();
    void updateStatus();
    void updateChannelStateForAllChannels();

    //QoS parameters
    uint32_t packetsOffered;
    uint32_t packetsServed;

public:
    //Exception classes
    class notInitialized{};

    user();
    user(trafficModel userTrafficModel, bool** channelStateForNeighbours, uint32_t* simulationTime);
    void initialize(trafficModel userTrafficModel, bool** channelStateForNeighbours, uint32_t* simulationTime);
    uint16_t getDataRate(uint8_t channelId);
    void transmitData(bool* channelsToBeUsed);
    uint32_t getBitsToSend();
    uint16_t getQueueLength();
    uint16_t getHoQPacketSize();
    uint32_t getHoQTTL();

    uint32_t getPacketsOffered();
    uint32_t getPacketsServed();

    //For Output purposes
    //friend ostream operator << (user u);
};

#endif // USER_H_INCLUDED
