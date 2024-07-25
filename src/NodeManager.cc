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

#include <map>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <queue>
#include <algorithm>
#include "NodeManager.h"
#include <random>

using namespace inet;

Define_Module(NodeManager);


void NodeManager::initialize(int stage){
    if (stage == INITSTAGE_LOCAL){
        communicationRange = par("communicationRange");
        groundStationRange = par("groundStationRange");
        destAddrs = par("destAddrs").stringValue();
        routeUpdateInterval = par("routeUpdateInterval");
        usableCommunicationRangeRatio = par("usableCommunicationRangeRatio"); // Initialize the usable communication range ratio
        initializeNetworkMsg = new cMessage("InitializeNetwork");
        scheduleAt(simTime(), initializeNetworkMsg);
        buildGraphMsg = new cMessage("BuildGraph");
        scheduleAt(simTime(), buildGraphMsg);

        // Calculate usable communication range
        communicationRange *= usableCommunicationRangeRatio;  // Update the communication range directly
        if (usableCommunicationRangeRatio < 1) {
            // Log the updated usable communication range
            EV_INFO << "Updated Usable Communication Range: " << communicationRange/1000.0 << " km." << endl; 
        }
            
        else {
            // Log the original communication range
            EV_INFO << "Using the original Communication Range: " << communicationRange/1000.0 << " km." << endl; 
        }   
    }
}

void NodeManager::recalculateRoutes() {
    positionOfRegisteredNodes.clear();
    ipAddressesOfRegisteredNodes.clear();
    std::vector<cModule *> activeNodes = checkActiveNodesAtTime();//working
    std::vector<Coord> activeNodesPosition = checkPositionsofActiveNodesAtTime(); //working
    std::vector<L3Address> activeNodesAddress = checkIPAddressofActiveNodesAtTime(); //working
    std::vector<L3Address> destAddresses;
    if (L3AddressResolver().tryResolve(destAddrs.c_str(), destAddress)) {
        EV << " Destination address is: " << destAddress << endl; //working
        destAddresses.push_back(destAddress);
    } else {
        EV << " Destination address not found! " << endl;
    }
    int destIdx = std::distance(activeNodesAddress.begin(), std::find(activeNodesAddress.begin(), activeNodesAddress.end(), destAddress));
    EV << "Destination Index is: " << destIdx << endl;//working
    destPosition = activeNodesPosition[destIdx];
    //EV << "Destination Position is: " << destPosition << endl;
    std::vector<std::vector<int>> adjacencyMatrix = BuildGraph(activeNodesPosition, communicationRange, destIdx, groundStationRange);//working 
    //destAddresses.push_back(ipAddressesOfRegisteredNodes[0]);
    printGraph(adjacencyMatrix);//working
    allShortetPaths.distances.clear();
    allShortetPaths.nextHops.clear();
    allShortPathsToDestinations.distances.clear();
    allShortPathsToDestinations.nextHops.clear();
    allShortetPaths = findAllShortestPaths(adjacencyMatrix,activeNodesAddress);
    allShortPathsToDestinations = findAllShortestPathsToDestination(adjacencyMatrix,activeNodesAddress,destAddresses);//working
    // printHopsforAllPaths();
    printRoutingTable(activeNodesAddress, destAddresses, allShortPathsToDestinations);
}

void NodeManager::registerClient(cModule* node){
    //check if the node is already registered to avoid duplicacy
    auto it = std::find(registeredNodes.begin(), registeredNodes.end(), node);
    if (it == registeredNodes.end()){
       EV << "Registering Client " << node->getIndex() << " ....\n"; //working
       registeredNodes.push_back(node);
       EV << "Number of Nodes: " << registeredNodes.size() << endl; //working
       EV << "Client " << node->getIndex() << " Registered!\n" << endl; //working
   }else{
       EV << "Client " << node->getIndex() << " is already registered.\n" << endl;
   }
}

void NodeManager::deregisterClient(cModule* node) {
   EV << "Deregistering Client " << node->getIndex() << " ....\n";
   auto it = std::find(registeredNodes.begin(), registeredNodes.end(), node);
   if (it != registeredNodes.end()) {
          registeredNodes.erase(it);
          EV << "Client " << node->getIndex() << " Deregistered!\n";
          // Check if the node is still present after deregistration
        it = std::find(registeredNodes.begin(), registeredNodes.end(), node);
        
        if (it != registeredNodes.end()) {
            EV << "Client " << node->getIndex() << " is still present after deregistration.\n";
        } else {
            EV << "Client " << node->getIndex() << " is no longer present after deregistration.\n";
        }
    } else {
        EV << "Client " << node->getIndex() << " not found for deregistration.\n";
    }
   if (simTime() > routeUpdateInterval) {
       // Print message indicating route update
       EV << "A new node is registered, updating the routes..." << endl;
       // Recalculate routes
       recalculateRoutes();
   } 
}

std::vector<cModule *>& NodeManager::checkActiveNodesAtTime()
{
    return registeredNodes;
}

std::vector<Coord>& NodeManager::checkPositionsofActiveNodesAtTime()
{
    int registeredNodesSize = registeredNodes.size();
    for (int i = 0; i < registeredNodesSize; ++i) {
        IMobility* mobility = check_and_cast<IMobility*>(registeredNodes[i]->getSubmodule("mobility"));
        Coord position = mobility->getCurrentPosition();
        positionOfRegisteredNodes.push_back(position);
    }
    return positionOfRegisteredNodes;
}

std::vector<L3Address>& NodeManager::checkIPAddressofActiveNodesAtTime()
{
    int registeredNodesSize = registeredNodes.size();
    for (int i = 0; i < registeredNodesSize; ++i) {
        L3Address address = L3AddressResolver().addressOf(registeredNodes[i]);
        ipAddressesOfRegisteredNodes.push_back(address);
    }
    return ipAddressesOfRegisteredNodes;
}

void NodeManager::handleMessage(cMessage* msg) {
    //std::vector<std::vector<int>> adjacencyMatrix = std::vector<std::vector<int>>(registeredNodes.size(), std::vector<int>(registeredNodes.size(), 0));
    if (msg == initializeNetworkMsg){
        EV << "Number of Nodes: " << registeredNodes.size() << endl; //working
        int registeredNodesSize = registeredNodes.size();
        for (int i = 0; i < registeredNodesSize; ++i) {
             IMobility* mobility = check_and_cast<IMobility*>(registeredNodes[i]->getSubmodule("mobility"));
             Coord position = mobility->getCurrentPosition();
             L3Address address = L3AddressResolver().addressOf(registeredNodes[i]);
             positionOfRegisteredNodes.push_back(position);
             ipAddressesOfRegisteredNodes.push_back(address);
        }

    }
    if (msg == buildGraphMsg){
        // positionOfRegisteredNodes.clear();
        // ipAddressesOfRegisteredNodes.clear();
        // std::vector<cModule *> activeNodes = checkActiveNodesAtTime();//working
        // std::vector<Coord> activeNodesPosition = checkPositionsofActiveNodesAtTime(); //working
        // std::vector<L3Address> activeNodesAddress = checkIPAddressofActiveNodesAtTime(); //working
        // std::vector<L3Address> destAddresses;
        // if (L3AddressResolver().tryResolve(destAddrs.c_str(), destAddress)) {
        //     EV << " Destination address is: " << destAddress << endl; //working
        //     destAddresses.push_back(destAddress);
        // } else {
        //     EV << " Destination address not found! " << endl;
        // }
        // int destIdx = std::distance(activeNodesAddress.begin(), std::find(activeNodesAddress.begin(), activeNodesAddress.end(), destAddress));
        // EV << "Destination Index is: " << destIdx << endl;//working
        // destPosition = activeNodesPosition[destIdx];
        // //EV << "Destination Position is: " << destPosition << endl;
        // std::vector<std::vector<int>> adjacencyMatrix = BuildGraph(activeNodesPosition, communicationRange, destIdx, groundStationRange);//working 
        // //destAddresses.push_back(ipAddressesOfRegisteredNodes[0]);
        // printGraph(adjacencyMatrix);//working
        // allShortetPaths.distances.clear();
        // allShortetPaths.nextHops.clear();
        // allShortPathsToDestinations.distances.clear();
        // allShortPathsToDestinations.nextHops.clear();
        // allShortetPaths = findAllShortestPaths(adjacencyMatrix,activeNodesAddress);
        // allShortPathsToDestinations = findAllShortestPathsToDestination(adjacencyMatrix,activeNodesAddress,destAddresses);//working
        // // printHopsforAllPaths();
        // printRoutingTable(activeNodesAddress, destAddresses, allShortPathsToDestinations);

        // scheduleAt(simTime() + 0.5, buildGraphMsg);
        // scheduleAt(simTime() + routeUpdateInterval, buildGraphMsg);
        // Recalculate routes
        recalculateRoutes();  
        scheduleAt(simTime() + routeUpdateInterval, buildGraphMsg);
    }
     else {
       EV << "Other Message received: " << msg << endl;
       delete msg;
    }
}

std::vector<std::vector<int>> NodeManager::BuildGraph(std::vector<Coord>& position, double communicationRange, int destIdx, double groundStationRange){
    int numNodes = position.size();
    // Coord destPosition = position[destIdx];
    std::vector<std::vector<int>> adjacencyMatrix(numNodes, std::vector<int>(numNodes, 0));
      for (int i = 0; i < numNodes; ++i) {
          for (int j = i + 1; j < numNodes; ++j) {
              if (i != j){
              double distance = position[i].distance(position[j]);
              // Determine the range to use based on the destination node
              double comm_range = (i == destIdx || j == destIdx) ? groundStationRange : communicationRange;
              if (distance <= comm_range) {
                  adjacencyMatrix[i][j] = 1;  // 1 indicates edge
                  adjacencyMatrix[j][i] = 1;
              }
              else {
                  adjacencyMatrix[i][j] = 0;
                  adjacencyMatrix[j][i] = 0;
              }
          }
      }
   }
      return adjacencyMatrix;
}

DijkstraAllPairsOutput NodeManager::findAllShortestPaths(std::vector<std::vector<int>>& adjacencyMatrix, std::vector<L3Address>& ipAddresses){
    int numNodes =  adjacencyMatrix.size();
    DijkstraAllPairsOutput result;
    result.distances = std::vector<std::vector<int>>(numNodes, std::vector<int>(numNodes, INT_MAX));
    result.nextHops = std::vector<std::vector<L3Address>>(numNodes, std::vector<L3Address>(numNodes));
    
    for (int src = 0; src < numNodes; ++src) {
        std::vector<int> dist(numNodes, INT_MAX);
        std::vector<int> previous(numNodes, -1);
        std::vector<bool> visited(numNodes, false);

        dist[src] = 0;

        using Pair = std::pair<int, int>; // Pair(distance, vertex)
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> Q;

        Q.push({ 0, src });

        while (!Q.empty()) {
            int u = Q.top().second;
            Q.pop();

            if (visited[u]) continue;
            visited[u] = true;

            for (int v = 0; v < numNodes; v++) {
                if (adjacencyMatrix[u][v] && !visited[v]) {
                    int alt = dist[u] + adjacencyMatrix[u][v];
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        previous[v] = u;
                        Q.push({ dist[v], v });
                    }
                }
            }
        }

        for (int i = 0; i < numNodes; i++) {
            result.distances[src][i] = dist[i];
            if (i == src) {
                result.nextHops[src][i] = ipAddresses[i];
            }
            else if (previous[i] != -1) {
                int nextHop = i;
                while (previous[nextHop] != src && previous[nextHop] != -1) {
                    nextHop = previous[nextHop];
                }
                result.nextHops[src][i] = ipAddresses[nextHop];
            }
        }
    }
    return result;

}

DijkstraAllPairsOutput NodeManager::findAllShortestPathsToDestination(std::vector<std::vector<int>>& adjacencyMatrix, std::vector<L3Address>& ipAddresses, std::vector<L3Address>& destinationIPAddresses){
    int numNodes =  adjacencyMatrix.size();
    int destSize = destinationIPAddresses.size();
    DijkstraAllPairsOutput result;
    result.distances = std::vector<std::vector<int>>(numNodes, std::vector<int>(numNodes, INT_MAX));
    result.nextHops = std::vector<std::vector<L3Address>>(numNodes, std::vector<L3Address>(numNodes));

    
    for (int src = 0; src < numNodes; ++src) {
        std::vector<int> dist(numNodes, INT_MAX);
        std::vector<int> previous(numNodes, -1);
        std::vector<bool> visited(numNodes, false);
        std::vector<L3Address> destinationIPAddressesTemp = destinationIPAddresses;
        dist[src] = 0;

        using Pair = std::pair<int, int>; // Pair(distance, vertex)
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> Q;

        Q.push({ 0, src });

        while (!Q.empty()) {
            int u = Q.top().second;
            Q.pop();

            if (visited[u]) continue;
            visited[u] = true;

            // Check for early termination condition
            auto it = std::find(destinationIPAddressesTemp.begin(), destinationIPAddressesTemp.end(), ipAddresses[u]);
            if (it != destinationIPAddressesTemp.end()) {
                destinationIPAddressesTemp.erase(it);
                if (destinationIPAddressesTemp.empty()) {
                    break;  // All desired destinations have been found
                }
            }

            for (int v = 0; v < numNodes; v++) {
                if (adjacencyMatrix[u][v] && !visited[v]) {
                    int alt = dist[u] + adjacencyMatrix[u][v];
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        previous[v] = u;
                        Q.push({ dist[v], v });
                    }
                }
            }
        }

        // Storing results only for specific destinations
        for (int j = 0; j < destSize; ++j) {
            int destIdx = std::distance(ipAddresses.begin(), std::find(ipAddresses.begin(), ipAddresses.end(), destinationIPAddresses[j]));
            
            result.distances[src][j] = dist[destIdx];
            if (previous[destIdx] != -1) {
                int nextHop = destIdx;
                while (previous[nextHop] != src && previous[nextHop] != -1) {
                    nextHop = previous[nextHop];
                }
                result.nextHops[src][j] = ipAddresses[nextHop];
            } else {
                result.nextHops[src][j] = ipAddresses[destIdx];
            }
        }
    }
    return result;
}


 void NodeManager::printRoutingTable(std::vector<L3Address>& ipAddresses, std::vector<L3Address>& destinationIPAddresses, DijkstraAllPairsOutput result){
    EV << "Routing Table:" << endl;
    EV << "Source IP | Destination IP | Next Hop IP | Hop Count" << endl;
    int ipAddressesSize = ipAddresses.size();
    for (int i = 0; i < ipAddressesSize; i++){
        int destinationIPAddressesSize = destinationIPAddresses.size();
        for(int j = 0; j < destinationIPAddressesSize; ++j){
            EV << ipAddresses[i] << " | " << destinationIPAddresses[j] << " | " << result.nextHops[i][j] << " | " << result.distances[i][j] << endl;
        }
    }
  }


 L3Address NodeManager::findNextHop(L3Address currentNodeAddress, L3Address destinationAddress)
 {
     
    //  // Recalculate routes
    //  recalculateRoutes();

     //EV << "Current Node Address is: " << currentNodeAddress << "\n"; //working
     L3Address nextHopAddress;

    int srcIdx = std::distance(ipAddressesOfRegisteredNodes.begin(), std::find(ipAddressesOfRegisteredNodes.begin(), ipAddressesOfRegisteredNodes.end(), currentNodeAddress)); 
    //EV << "Source Index is: " << srcIdx << endl;//working
    int destIdx = std::distance(ipAddressesOfRegisteredNodes.begin(), std::find(ipAddressesOfRegisteredNodes.begin(), ipAddressesOfRegisteredNodes.end(), destinationAddress)); 
    //EV << "Destination Index is: " << destIdx << endl;//working
    int ipAddressesOfRegisteredNodesSize = ipAddressesOfRegisteredNodes.size();
    if (srcIdx >= 0 && srcIdx < ipAddressesOfRegisteredNodesSize && destIdx >= 0  && destIdx < ipAddressesOfRegisteredNodesSize){
        nextHopAddress = allShortetPaths.nextHops[srcIdx][destIdx];
        EV << "Next Hop Address is: " << nextHopAddress << endl;
        return nextHopAddress;
    }

    // Return an invalid address if source or destination is not found.
    return inet::L3Address();
 }


void NodeManager::printGraph(std::vector<std::vector<int>> graph) {
     int numNodes =  graph.size();
     EV << "Graph matrix:" << endl;
     for (int i = 0; i < numNodes; ++i) {
         for (int j = 0; j < numNodes; ++j) {
             EV << graph[i][j] << " ";
         }
         EV << endl;
     }
 }

