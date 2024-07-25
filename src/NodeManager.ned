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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
package dspr;

module NodeManager
{
    parameters:
       @display("i=block/routing");
       double communicationRange @unit(m) = default(0m);
       //string communicationRange @unit(m) = default("100m 200m 300m 400m 500m");
       double groundStationRange @unit(m) = default(0m);
       string destAddrs =  default("groundStation[0]");  
       double routeUpdateInterval @unit(s) = default(0.5s); // Interval for route updates in seconds
       double usableCommunicationRangeRatio = default(1.0); // Ratio of usable communication range, default is 1 (full range)
       @class(NodeManager);
       string interfaces = default("wlan0");
      
    //gates:
      // input in;
       //output out;
    
}