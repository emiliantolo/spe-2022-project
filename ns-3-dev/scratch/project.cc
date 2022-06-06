#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h" // ce ne sono diversi
#include "ns3/ssid.h" //for ssid in wireless network
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"

// WIRELESS NETWORK

// Default Network Topology
//
//  Wifi 10.1.3.0
//			 AP
//  *  *  *  *
//  |  |  |  | 
//                 10.1.1.0
//  n5 n6 n7 n0 -------------- n1 	n2	n3	n4
//             point-to-point   |	  |	  |	  |
//					                     ===========
//					                    LAN 10.1.2.0
 
// n0 --> access point 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LaboratoryExample");

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3; // 3 + access point
  
  CommandLine cmd (__FILE__);
  
  cmd.AddValue ("nCsma", "number of extra CSMA nodes", nCsma);
  cmd.AddValue ("nWifi", "number of wifi STA nodes", nWifi);
  cmd.AddValue ("verbose", "Enable logging", verbose);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  
  if (verbose) {
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }
  
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);
  
  NodeContainer wifiStaNodes; //container with Station
  wifiStaNodes.Create(nWifi);

  NodeContainer wifiApNodes = p2pNodes.Get(0); 

  //wifi property 
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetErrorRateModel ("ns3::NistErrorRateModel");
  phy.SetChannel (channel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  WifiMacHelper mac;

  Ssid ssid = Ssid("spe-ssid");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifiStaNodes);
  
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNodes);
  
  MobilityHelper mobility; //mobilità dei nodi wireless

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  "MinX", DoubleValue (0.0),
  "MinY", DoubleValue (0.0),
  "DeltaX", DoubleValue (5.0),
  "DeltaY", DoubleValue (10.0),
  "GridWidth", UintegerValue (3),
  "LayoutType", StringValue ("RowFirst"));
  
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  
  mobility.Install (wifiStaNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);
    
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);  
  
  address.SetBase ("10.1.2.0","255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0","255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces = address.Assign (staDevices);
  wifiInterfaces = address.Assign (apDevices);
  
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // installation of application
  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //enable routing table
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  Simulator::Stop (Seconds(10.0));
  
  //enable sniffing
  pointToPoint.EnablePcapAll ("lab2");
  csma.EnablePcap("lab2", csmaDevices.Get (1), true); //first lan node (node 1)
  phy.EnablePcap("lab2", apDevices.Get (0));  //access point wireless (node 0)

  /*  rm lab2-*
      ./waf --run scratch/lab2-3.cc
      ls lab2-* --> file sniffing 
      tcpdump -n -t -r lab2-0-0.pcap
  */

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

