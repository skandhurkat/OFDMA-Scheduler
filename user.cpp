//user.cpp
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

#include <cmath>
#include "user.h"
#include "random.h"
#include "constants.h"
#include "packet.h"
using namespace std;

const uint16_t user::CQI2DataRate[] = {0, 70, 108, 174, 278, 405, 544, 683, 885, 1113, 1263, 1537, 1806, 2093, 2367, 2571};   //Assuming 0.5 ms TTI, and using Table 7.2.3-1: 4-bit CQI Table from 36213-a40. Data rate = 6/7*15000kSps*0.5ms/S*12subcarriers/RB*6RB/channel*efficiency
const float user::beta[] = {5, 5.01, 5.01, 0.84, 1.67, 1.61, 1.64, 3.87, 5.06, 6.40, 12.59, 17.59, 23.33, 29.45, 33.05, 35.41};
const float user::SINRthresh[] = {0.2026, 0.3507, 0.4808, 0.7492, 1.1915, 1.8621, 2.9492, 4.4957, 7.2044, 10.8893, 16.9824, 26.1818, 38.8150, 60.5341, 96.1612};

void user::placeUser()
{
    if(!initialized) throw(notInitialized());

    float r = sqrt((pow(MAX_DIST,2)-pow(MIN_DIST,2))*uniformRand()+pow(MIN_DIST,2));
//    float r = MAX_DIST;             //DEBUG ONLY. REVERT TO RANDOM PLACEMENT.
    float theta = 2*PI*uniformRand();
    position.x = r*cos(theta);
    position.y = r*sin(theta);
    for(int i=0; i<7; i++)
    {
        shadowingParams[i] = pow(10,normalRand(0,SHADOWING_SIGMA));
    }
    timeShadowingLastUpdated = *simulationTime;
}

void user::generateTraffic()
{
    if(!initialized) throw(notInitialized());
    else if(userTrafficModel == BURSTY)
    {
        uint16_t numPackets = poissonRand(0.4);
        for(int packets=0; packets<numPackets; packets++)
        {
            packet pkt;
            pkt.size = 500;
            bitsInQueue += pkt.size;
            pkt.expiryTime = (*simulationTime)+160;
            dataQueue.push(pkt);
        }
        packetsOffered += numPackets;
    }
}

void user::updateStatus()
{
    if(!initialized) throw(notInitialized());
    if(*simulationTime > timeLastUpdated)
        updateChannelStateForAllChannels();
    while(*simulationTime > timeLastUpdated)    //Generate traffic
    {
        generateTraffic();
        timeLastUpdated++;
    }
    while(!dataQueue.empty())                   //Remove expired packets
    {
        if((dataQueue.top()).expiryTime < *simulationTime)
        {
            bitsInQueue -= (dataQueue.top()).size;
            dataQueue.pop();
        }
        else break;
    }
}

void user::updateChannelStateForAllChannels()
{
    if(!initialized) throw(notInitialized());

    coordinates neighbourCoordinates[6];
    for(int neighbouringCell=0; neighbouringCell<6; neighbouringCell++)
    {
        neighbourCoordinates[neighbouringCell].x = 2*CELL_RADIUS*cos(PI*neighbouringCell/3);
        neighbourCoordinates[neighbouringCell].y = 2*CELL_RADIUS*sin(PI*neighbouringCell/3);
    }

    if(*simulationTime-timeShadowingLastUpdated > UPDATE_SHADOWING_AFTER)
    {
        for(int i=0; i<7; i++)
            shadowingParams[i] = pow(10,normalRand(0,SHADOWING_SIGMA));
        timeShadowingLastUpdated = *simulationTime;
    }

    for(int channel=0; channel<NUM_CHANNELS; channel++)
    {
        uint8_t CQI=15;
        float SINR[6];
        float SINReff;
        float interference[6];      //A hack to save time. It stores the raw interference from adjacent cells, without rayleigh fading. Multiply with an exponential random variable to get the rayleigh faded values.
        float signalStrength = TRANSMIT_POWER*shadowingParams[0]/pow((pow(position.x,2)+pow(position.y,2)),DIST_ATTENUATING_FACTOR/2);
        for(int neighbouringCell=0; neighbouringCell<6; neighbouringCell++)
        {
            if(channelStateForNeighbours[channel][neighbouringCell])
            {
                interference[neighbouringCell] = TRANSMIT_POWER*shadowingParams[neighbouringCell+1]/pow((pow(position.x-neighbourCoordinates[neighbouringCell].x,2)+pow(position.y-neighbourCoordinates[neighbouringCell].y,2)),DIST_ATTENUATING_FACTOR/2);
            }
            else interference[neighbouringCell] = 0;
        }
        for(int rb=0; rb < 6; rb++)
        {
            SINR[rb] = signalStrength*exponentialRand(1/MEAN_RAYLEIGH_FADING)/(interference[0]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+interference[1]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+interference[2]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+interference[3]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+interference[4]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+interference[5]*exponentialRand(1/MEAN_RAYLEIGH_FADING)+THERMAL_NOISE);
        }
        while(CQI>0)
        {
            float dummy = exp(-SINR[0]/beta[CQI])+exp(-SINR[1]/beta[CQI])+exp(-SINR[2]/beta[CQI])+exp(-SINR[3]/beta[CQI])+exp(-SINR[4]/beta[CQI])+exp(-SINR[5]/beta[CQI]);
            if(dummy < 10E-21)
            {
                SINReff = 10000;
                break;
            }
            SINReff = -beta[CQI]*log(1.0/6.0*dummy);
            if(SINReff > SINRthresh[CQI-1]) break;
            CQI--;
        }
        dataRate[channel] = CQI2DataRate[CQI];
    }
}

user::user(trafficModel userTrafficModel, bool** channelStateForNeighbours, uint32_t* simulationTime): initialized(true), simulationTime(simulationTime), timeLastUpdated(0), timeShadowingLastUpdated(0), bitsInQueue(0), userTrafficModel(userTrafficModel), channelStateForNeighbours(channelStateForNeighbours), packetsOffered(0), packetsServed(0)
{
    placeUser();
    updateStatus();
}

user::user(): initialized(false), simulationTime(NULL), timeLastUpdated(0), timeShadowingLastUpdated(0), bitsInQueue(0), userTrafficModel(BURSTY), channelStateForNeighbours(NULL), packetsOffered(0), packetsServed(0)
{
    //do nothing
}

void user::initialize(trafficModel userTrafficModel, bool** channelStateForNeighbours, uint32_t* simulationTime)
{
    this->userTrafficModel = userTrafficModel;
    this->channelStateForNeighbours = channelStateForNeighbours;
    this->simulationTime = simulationTime;
    initialized = true;
    placeUser();
    updateStatus();
}

uint16_t user::getDataRate(uint8_t channelId)
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return dataRate[channelId];
}

void user::transmitData(bool* channelsToBeUsed)
{
    if(!initialized) throw(notInitialized());
    uint32_t bitsToSend = 0;
    updateStatus();
    for(int i=0; i<NUM_CHANNELS; i++)
        if(channelsToBeUsed[i])
            bitsToSend += dataRate[i];

    while(!dataQueue.empty() && (dataQueue.top()).size < bitsToSend)
    {
        bitsToSend -= (dataQueue.top()).size;
        bitsInQueue -= (dataQueue.top()).size;
        dataQueue.pop();
        packetsServed++;
    }
}

uint32_t user::getBitsToSend()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return bitsInQueue;
}

uint16_t user::getQueueLength()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return dataQueue.size();
}

uint16_t user::getHoQPacketSize()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return dataQueue.empty() ? 0 : dataQueue.top().size;
}

uint32_t user::getHoQTTL()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return dataQueue.empty() ? ~static_cast<uint32_t>(0) : dataQueue.top().expiryTime-*(simulationTime);
}

uint32_t user::getPacketsOffered()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return packetsOffered;
}

uint32_t user::getPacketsServed()
{
    if(!initialized) throw(notInitialized());
    updateStatus();
    return packetsServed;
}
