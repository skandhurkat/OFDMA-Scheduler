//ProportionalFair.cpp
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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <getopt.h>
#include "user.h"
#include "random.h"

using namespace std;

string program_name;

void usage()
{
    cout << "Usage is of the form " << program_name << " [options]" << endl
        << "Options are:" << endl
        << "--seed <seed value for random number generator>"
        << "--help" << endl;
}

int main(int argc, char** argv)
{
    program_name = argv[0];
    const char* const short_options = "s:h";
    static const struct option long_options[] =
    {
        {"seed", required_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    uint32_t seed = time(NULL);

    int opt;
    while((opt=getopt_long(argc,argv,short_options,long_options,NULL)) != -1)
    {
        switch(opt)
        {
        case 's':   seed = atol(optarg);
                    break;
        default:    usage();
        }
    }

    char outFileName[40] = "Proportional_Fair_160-";
    char cSeed[11];
//    strcpy(outFileName, "Indexing-");
    ltoa(seed, cSeed, 10);
    strcat(outFileName, cSeed);
    strcat(outFileName, ".csv");
    seedRand(seed);
    uint32_t simTime=1;
    static const int NUM_USERS = 10;         //10 users for good diversity

    bool **CSN;
    CSN = new bool *[14];
    for(int i = 0; i<14;i++) // need to define the dimension of Channel state for Neigh. matrix
    {
        CSN[i] = new bool [6];
    }
    for(int i = 0; i<14;i++)
    {
        for(int j=0;j<6;j++)
        {
            CSN[i][j] = binaryRand(0.72);
        }
    }

    ofstream outFile(outFileName);
    outFile << "Sim Time";

    user userArray[NUM_USERS];
    for(int i=0; i<NUM_USERS; i++)
    {
        userArray[i].initialize(BURSTY, CSN, &simTime);
        for(int j=0; j<14; j++)
        {
            outFile << ",user " << i << " channel " << j;
        }
        outFile << ",user " << i << " packets offered," << "user " << i << " packets served," << "user " << i << " packets dropped"
                << ",user " << i << " queue length";
    }
    outFile << endl;

    seedRand();

    //Proportional fair scheduling
    static const int NUM_CHANNELS = 2;
    float averageRate[NUM_USERS];
    for(int i=0; i<NUM_USERS; i++)
    {
        averageRate[i] = 1.0;
    }
    float delta[NUM_USERS][NUM_CHANNELS];
    static const float c = 0.05;
    for(int k=0;k<100000;k++)
    {
        for(int i = 0; i<14;i++)    //Update channel state for neighbours
        {
            for(int j=0;j<6;j++)
            {
                CSN[i][j] = binaryRand(0.72);
            }
        }

        for(int i=0; i<NUM_CHANNELS; i++)
        {
            for(int j=0; j<NUM_USERS; j++)
            {
                delta[j][i] = static_cast<float>(userArray[j].getDataRate(i))/averageRate[j];// * exp(static_cast<float>(userArray[j].getQueueLength())/50) * exp(-static_cast<float>(userArray[j].getHoQTTL())/20);
            }
        }
        int channelAllocationVector[NUM_CHANNELS];
        for(int i=0; i<NUM_CHANNELS; i++)
        {
            channelAllocationVector[i] = 0;
            for(int j=1; j<NUM_USERS; j++)
            {
                if(delta[j][i] > delta[(channelAllocationVector[i])][i]) channelAllocationVector[i] = j;
            }
        }

        for(int j=0; j<NUM_USERS; j++)
        {
            bool transmitVector[14];
            for(int i=0; i<14; i++)
                transmitVector[i] = false;
            float userDataRate = 0;

            for(int i=0; i<NUM_CHANNELS; i++)
            {
                if(channelAllocationVector[i] == j)
                {
                    userDataRate += userArray[j].getDataRate(i);
                    transmitVector[i] = true;
                }
            }
            userArray[j].transmitData(transmitVector);
            averageRate[j] = averageRate[j]*(1-c) + userDataRate*c;
        }

//End of PF Scheduler

        outFile << simTime;
        for(int i=0; i<NUM_USERS; i++)
        {
            for(int j=0; j<14; j++)
            {
                outFile << ',' << userArray[i].getDataRate(j);
            }
            outFile << ',' << userArray[i].getPacketsOffered() << ',' << userArray[i].getPacketsServed()
                    << ',' << userArray[i].getPacketsOffered()-(userArray[i].getPacketsServed()+userArray[i].getQueueLength())
                    << ',' << userArray[i].getQueueLength();
        }
        outFile << endl;
        simTime++;
    }
    ofstream tempFile("temp.csv");
    tempFile << userArray[0].getPacketsOffered() << ',' << userArray[0].getPacketsOffered()-(userArray[0].getPacketsServed()+userArray[0].getQueueLength());
    for(int i=1; i<NUM_USERS; i++)
        tempFile << ',' << userArray[i].getPacketsOffered() << ',' << userArray[i].getPacketsOffered()-(userArray[i].getPacketsServed()+userArray[i].getQueueLength());
    return 0;
}
