//packet.h
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

#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <inttypes.h>
using namespace std;

class packet
{
public:
    uint16_t size;
    uint32_t expiryTime;

    packet(): size(0), expiryTime(0) {}
    bool operator < (const packet& pkt) const
    {
        return expiryTime > pkt.expiryTime;     //This may appear contradictory, but packets are placed in a priority queue. Prority is higher for packets with an earlier deadline.
    }
};

#endif // PACKET_H_INCLUDED
