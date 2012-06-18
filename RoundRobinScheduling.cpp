//RoundRobinScheduling.cpp
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

    char outFileName[30] = "RoundRobin_120-";
    char cSeed[11];
//    strcpy(outFileName, "Indexing-");
    ltoa(seed, cSeed, 10);
    strcat(outFileName, cSeed);
    strcat(outFileName, ".csv");
    seedRand(seed);
    uint32_t simTime=1;

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

    user userArray[10];         //10 users for good diversity
    for(int i=0; i<10; i++)
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
    seedRand();             //Quick and dirty fix to ensure randomness in results, but with same position of users.

    //Pure round robin scheduling with no intelligence whatsoever.
    int userBeingScheduled = 0;
    bool schedulingArray1[]={true, false, false, false, false, false, false, false, false, false, false, false, false, false};
    bool schedulingArray2[]={false, true, false, false, false, false, false, false, false, false, false, false, false, false};
    for(int k=0;k<100000;k++)
    {
        for(int i = 0; i<14;i++)    //Update channel state for neighbours
        {
            for(int j=0;j<6;j++)
            {
                CSN[i][j] = binaryRand(0.72);
            }
        }
        userArray[userBeingScheduled].transmitData(schedulingArray1);
        userBeingScheduled = (userBeingScheduled+1)%10;
        userArray[userBeingScheduled].transmitData(schedulingArray2);

        outFile << simTime;
        for(int i=0; i<10; i++)
        {
            for(int j=0; j<14; j++)
            {
                outFile << ',' << userArray[i].getDataRate(j);
            }
            outFile << ',' << userArray[i].getPacketsOffered() << ',' << userArray[i].getPacketsServed() << ','
                    << userArray[i].getPacketsOffered() - (userArray[i].getPacketsServed()+userArray[i].getQueueLength())
                    << ',' << userArray[i].getQueueLength();
        }
        outFile << endl;
        simTime++;
    }
    ofstream tempFile("temp.csv");
    tempFile << userArray[0].getPacketsOffered() << ',' << userArray[0].getPacketsOffered()-(userArray[0].getPacketsServed()+userArray[0].getQueueLength());
    for(int i=1; i<10; i++)
        tempFile << ',' << userArray[i].getPacketsOffered() << ',' << userArray[i].getPacketsOffered()-(userArray[i].getPacketsServed()+userArray[i].getQueueLength());

    return 0;

//    uint32_t simulationTime = 0;
//    bool channelStateForNeighbours[14][6];
//    user users[100];
//    return 0;
}
