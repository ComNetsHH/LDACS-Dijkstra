// The LDACS Dijkstra implements the Dijkstra's shortest path routing algorithm.
// Copyright (C) 2024  Musab Ahmed, Sohini Maji, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef NODEMANAGER_H_
#define NODEMANAGER_H_

#include <map>
#include <vector>
#include <queue>
#include <tuple>
#include "inet/common/INETDefs.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include "Dspr.h"
#include "inet/networklayer/common/L3Address.h"

using namespace omnetpp;
using namespace std;
using namespace inet;


struct DijkstraAllPairsOutput {
    std::vector<std::vector<int>> distances;
    std::vector<std::vector<L3Address>> nextHops;
};


class Dspr;

class NodeManager : public cSimpleModule {

protected:
   Dspr* dspr = nullptr;
   cMessage* initializeNetworkMsg = nullptr;
   cMessage* buildGraphMsg = nullptr;
   double communicationRange;
   // std::vector<double> communicationRange;
   double groundStationRange;
   std::string destAddrs;
   //int destIndex;
   std::vector<std::tuple<L3Address, L3Address, L3Address, int>> routingTable;
   simtime_t routeUpdateInterval; // Interval for route updates in seconds 
   double usableCommunicationRangeRatio;

   virtual void initialize(int stage) override;
   virtual void handleMessage(cMessage *msg) override;

public:
   std::vector<cModule*> registeredNodes;
   std::vector<L3Address> ipAddressesOfRegisteredNodes;
   std::vector<Coord> positionOfRegisteredNodes;
   DijkstraAllPairsOutput allShortetPaths;
   DijkstraAllPairsOutput allShortPathsToDestinations;
   L3Address srcIpAddress;
   L3Address destAddress;
   Coord destPosition;
   //Node initialization
   void registerClient(cModule* node); 
   void deregisterClient(cModule* node);
   void recalculateRoutes();

   //Finding node details at current time
   std::vector<cModule*>& checkActiveNodesAtTime();
   std::vector<Coord>& checkPositionsofActiveNodesAtTime();
   std::vector<L3Address>& checkIPAddressofActiveNodesAtTime();

   //Algorithm
   std::vector<std::vector<int>> BuildGraph(std::vector<Coord>& position, double communicationRange, int destIdx, double groundStationRange);
   DijkstraAllPairsOutput findAllShortestPaths(std::vector<std::vector<int>>& adjacencyMatrix, std::vector<L3Address>& ipAddresses);
   DijkstraAllPairsOutput findAllShortestPathsToDestination(std::vector<std::vector<int>>& adjacencyMatrix, std::vector<L3Address>& ipAddresses, std::vector<L3Address>& destinationIPAddresses);
   L3Address findNextHop(L3Address currentNodeAddress, L3Address destinationAddress);

   //printing functions
   void printRoutingTable(std::vector<L3Address>& ipAddresses, std::vector<L3Address>& destinationIPAddresses, DijkstraAllPairsOutput result);
   void printGraph(std::vector<std::vector<int>> graph);

};

#endif /* NODEMANAGER_H_ */
