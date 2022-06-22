/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/ipv4-flow-classifier.h"
#include <fstream>
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/tcp-socket.h"



// Network topology:
//  STA   AP  STA
//   *    *    *
//   |    |    |
//   n0   n2   n1

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Homework5");

void run ( bool enableRTS, bool ls)
{
    // 1. Initialization
    uint32_t nSta = 2;
    uint32_t nAp = 1;
    uint64_t simulation_Time = 10;
    double distance = 150;
  
  
  if(enableRTS==false)
  {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
  }
  else
  {
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  }
  
  // 2. Creation of the Nodes
  NodeContainer wifiStaNodes;
  NodeContainer wifiApNode;
  wifiApNode.Create (nAp);
  wifiStaNodes.Create (nSta);
    // 2.1 Set Position of the Nodes
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      positionAlloc->Add (Vector (distance, 0.0, 0.0));
      positionAlloc->Add (Vector (0.0, 0.0, 0.0));
      positionAlloc->Add (Vector ((distance*2), 0.0, 0.0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiApNode);
      mobility.Install (wifiStaNodes);
    
      std::cout<<"\n ====== WIFI BASE STATION Setup: ======\n";
    
      for (NodeContainer::Iterator j = wifiStaNodes.Begin ();j != wifiStaNodes.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
       std::cout<<"\n ====== ACCESS POINT Setup: ======\n";
       for (NodeContainer::Iterator j = wifiApNode.Begin ();j != wifiApNode.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
        
        std::cout << "-----------------------\n" ;
      
      
      // 3. Creation and setup of the channel
        Ptr<MatrixPropagationLossModel> lossModel;
        // Create channel
        Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
        
        if(ls==true){
          Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
          lossModel->SetDefaultLoss(200);
          lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
            
            wifiChannel->SetPropagationLossModel (lossModel);
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
        }else if(ls==false){
            Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
            log->SetAttribute ("ReferenceDistance", DoubleValue (150));
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
            wifiChannel->SetPropagationLossModel (log);
            
        }

     // 4. Install wireless devices
      WifiHelper wifi;
      wifi.SetStandard (WIFI_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
      
      YansWifiPhyHelper phy;
      phy.SetChannel (wifiChannel);
      
      WifiMacHelper macSTA;
      macSTA.SetType ("ns3::StaWifiMac");
      NetDeviceContainer staDevices;
      staDevices = wifi.Install (phy, macSTA, wifiStaNodes);
      WifiMacHelper macAP;
      macAP.SetType ("ns3::ApWifiMac");
      NetDeviceContainer apDevice;
      apDevice = wifi.Install (phy, macAP, wifiApNode);
    
      
     // 5. Install TCP/IP stack & assign IP addresses
      InternetStackHelper stack;
      stack.Install (wifiApNode);
      stack.Install (wifiStaNodes);
      
      Ipv4AddressHelper address;
      address.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer ApInterface;
      ApInterface = address.Assign (apDevice);
      Ipv4InterfaceContainer StaInterface;
      StaInterface = address.Assign (staDevices);
      
      std::cout << " ====== ADRESSES ====== \n" ;
      std::cout << " STA1: " << StaInterface.GetAddress (0) << "\n";
      std::cout << " STA2: " << StaInterface.GetAddress (1) << "\n";
      std::cout << " AP: " << ApInterface.GetAddress (0) << "\n";
      std::cout << "-----------------------\n" ;
      
     // 6. Install applications: two CBR streams
      ApplicationContainer cbrApps;
      OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (0),12345));
      onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      
      
          // Flow 1: n0 -> n2
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (0)));
          
          // Flow 2: n1 -> n2
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps")); //changed
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0))); //changed
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (1)));
          
          UdpEchoClientHelper client (ApInterface.GetAddress (0) , 9);
          client.SetAttribute ("MaxPackets", UintegerValue (10));
          client.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps;
          
          UdpEchoClientHelper client1 (ApInterface.GetAddress (0) , 9);
          client1.SetAttribute ("MaxPackets", UintegerValue (5));
          client1.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client1.SetAttribute ("PacketSize", UintegerValue (1024));
          ApplicationContainer pingApps1;
      
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (0)));
      
      client1.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps1.Add (client1.Install (wifiStaNodes.Get (1)));
      
      // 7. Flow monitor
      FlowMonitorHelper flowHelper;
      Ptr<FlowMonitor> flowMonitor;
      flowMonitor = flowHelper.InstallAll();
      
      Simulator::Stop (Seconds(simulation_Time));
      Simulator::Run ();
      
      flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);
      
      flowMonitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
      
      std::cout << "-----------------------\n" ;
      
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
          if ( i->first > 2)
          {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
              std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
              std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
              std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
              std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
              std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

              std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
              std::cout<<"========================================================================================\n";
              std::cout << "  Retransmissions: " << (i->second.txPackets - i->second.rxPackets) << " \n";
              std::cout<<"========================================================================================\n";
            }
        }
        
      // 8. Enable routing table
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Simulator::Stop (Seconds (simulation_Time));
       
     // 9. Enable sniffing
     phy.EnablePcapAll ("Homework5");
     phy.EnablePcap ("AP", apDevice.Get (0));
     phy.EnablePcap ("STA1", staDevices.Get (0));
     phy.EnablePcap ("STA2", staDevices.Get (1));
     
    
     
     Simulator::Run ();
     Simulator::Destroy ();
    

   }


void run_exposed_problem ( bool enableRTS)
{
    // 1. Initialization
    uint32_t nSta = 2;
    uint32_t nAp = 2;
    uint64_t simulation_Time = 10;
    //double distance = 150;
  
  
  if(!enableRTS)
  {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
  }
  else
  {
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  }
  
  // 2. Creation of the Nodes
  NodeContainer wifiStaNodes;
  NodeContainer wifiApNodes;
  wifiApNodes.Create (nAp);
  wifiStaNodes.Create (nSta);
    // 2.1 Set Position of the Nodes
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    /*
     positionAlloc->Add (Vector (0.0, 0.0, 0.0));
     positionAlloc->Add (Vector ((distance*3), 0.0, 0.0));
     positionAlloc->Add (Vector (distance, 0.0, 0.0));
     positionAlloc->Add (Vector ((distance*2), 0.0, 0.0));
     */
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector ((300), 0.0, 0.0));
    positionAlloc->Add (Vector (100, 0.0, 0.0));
    positionAlloc->Add (Vector ((200), 0.0, 0.0));
      
     
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiApNodes);
      mobility.Install (wifiStaNodes);
    
      std::cout<<"\n ====== WIFI BASE STATION Setup: ======\n";
    
      for (NodeContainer::Iterator j = wifiStaNodes.Begin ();j != wifiStaNodes.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
       std::cout<<"\n ====== ACCESS POINT Setup: ======\n";
       for (NodeContainer::Iterator j = wifiApNodes.Begin ();j != wifiApNodes.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
        
        std::cout << "-----------------------\n" ;
      
      
      // 3. Creation and setup of the channel
        Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
        
            Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
            //log->SetAttribute("Exponent", DoubleValue(2.5));
            log->SetAttribute ("ReferenceDistance", DoubleValue (100));
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
            wifiChannel->SetPropagationLossModel (log);
            
        

     // 4. Install wireless devices
      WifiHelper wifi;
      wifi.SetStandard (WIFI_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
      
      YansWifiPhyHelper phy;
      phy.SetChannel (wifiChannel);
      
      WifiMacHelper macSTA;
      macSTA.SetType ("ns3::StaWifiMac");
      NetDeviceContainer staDevices;
      staDevices = wifi.Install (phy, macSTA, wifiStaNodes);
      WifiMacHelper macAP;
      macAP.SetType ("ns3::ApWifiMac");
      NetDeviceContainer apDevice;
      apDevice = wifi.Install (phy, macAP, wifiApNodes);
    
      
     // 5. Install TCP/IP stack & assign IP addresses
      InternetStackHelper stack;
      stack.Install (wifiApNodes);
      stack.Install (wifiStaNodes);
      
      Ipv4AddressHelper address;
      address.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer ApInterface;
      ApInterface = address.Assign (apDevice);
      Ipv4InterfaceContainer StaInterface;
      StaInterface = address.Assign (staDevices);
      
      std::cout << " ====== ADRESSES ====== \n" ;
      std::cout << " STA1: " << StaInterface.GetAddress (0) << "\n";
      std::cout << " STA2: " << StaInterface.GetAddress (1) << "\n";
      std::cout << " AP1: " << ApInterface.GetAddress (0) << "\n";
      std::cout << " AP2: " << ApInterface.GetAddress (1) << "\n";
      std::cout << "-----------------------\n" ;
      
     // 6. Install applications: two CBR streams
      ApplicationContainer cbrApps;
      OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (0),12345));
      OnOffHelper onOffHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (1),12345));
      onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onOffHelper1.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
      
      
          // Flow 1: n0 -> n2
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (0)));
          
          // Flow 2: n1 -> n3
          onOffHelper1.SetAttribute ("DataRate", StringValue ("6000000bps")); //changed
          onOffHelper1.SetAttribute ("StartTime", TimeValue (Seconds (1.0))); //changed
          cbrApps.Add (onOffHelper1.Install (wifiStaNodes.Get (1)));
          
          /*UdpEchoClientHelper client (ApInterface.GetAddress (0) , 9);
          client.SetAttribute ("MaxPackets", UintegerValue (10));
          client.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps;
          
          UdpEchoClientHelper client1 (ApInterface.GetAddress (1) , 9);
          client1.SetAttribute ("MaxPackets", UintegerValue (5)); //changed
          client1.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client1.SetAttribute ("PacketSize", UintegerValue (1024));
          ApplicationContainer pingApps1;*/
      
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (0)));
      
      client1.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps1.Add (client1.Install (wifiStaNodes.Get (1)));
      
      // 7. Flow monitor
      FlowMonitorHelper flowHelper;
      Ptr<FlowMonitor> flowMonitor;
      flowMonitor = flowHelper.InstallAll();
      
      Simulator::Stop (Seconds(simulation_Time));
      Simulator::Run ();
      
      flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);
      
      flowMonitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
      
      std::cout << "-----------------------\n" ;
      
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
          if ( i->first > 2)
          {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
              std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
              std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
              std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
              std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
              std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

              std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
              std::cout<<"========================================================================================\n";
              std::cout << "  Retransimissions: " << (i->second.txPackets - i->second.rxPackets) << " \n";
              std::cout<<"========================================================================================\n";
            }
        }
        
      // 8. Enable routing table
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Simulator::Stop (Seconds (simulation_Time));
       
     // 9. Enable sniffing
     phy.EnablePcapAll ("Homework5");
     phy.EnablePcap ("AP1", apDevice.Get (0));
     phy.EnablePcap ("AP2", apDevice.Get (1));
     phy.EnablePcap ("STA1", staDevices.Get (0));
     phy.EnablePcap ("STA2", staDevices.Get (1));
     
    
     
     Simulator::Run ();
     Simulator::Destroy ();
    

   }

/*
void run_more_nodes ( bool enableRTS, bool ls)
{
    // 1. Initialization
    uint32_t nSta = 3;
    uint32_t nAp = 3;
    uint64_t simulation_Time = 20;
    double distance = 150;
  
  
  if(enableRTS==false)
  {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
  }
  else
  {
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  }
  
  // 2. Creation of the Nodes
  NodeContainer wifiStaNodes;
  NodeContainer wifiApNode;
  wifiApNode.Create (nAp);
  wifiStaNodes.Create (nSta);
    // 2.1 Set Position of the Nodes
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    //AP
      positionAlloc->Add (Vector (distance, 0.0, 0.0));
      positionAlloc->Add (Vector ((distance/2), 129.95, 0.0));
      positionAlloc->Add (Vector ((1.5)*distance, 129.95, 0.0));
    //STA
      positionAlloc->Add (Vector (0.0, 0.0, 0.0));
      positionAlloc->Add (Vector (distance*2, 0.0, 0.0));
      positionAlloc->Add (Vector (distance, 259.9, 0.0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiApNode);
      mobility.Install (wifiStaNodes);
    
      std::cout<<"\n ====== WIFI BASE STATION Setup: ======\n";
    
      for (NodeContainer::Iterator j = wifiStaNodes.Begin ();j != wifiStaNodes.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
       std::cout<<"\n ====== ACCESS POINT Setup: ======\n";
       for (NodeContainer::Iterator j = wifiApNode.Begin ();j != wifiApNode.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
        
        std::cout << "-----------------------\n" ;
      
      
      // 3. Creation and setup of the channel
        Ptr<MatrixPropagationLossModel> lossModel;
        // Create channel
        Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
        
        if(ls==true){
          Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
          lossModel->SetDefaultLoss(200);
          lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (1)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (2)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (2)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (1)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (1)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (2)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
            
            wifiChannel->SetPropagationLossModel (lossModel);
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
        }else if(ls==false){
            Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
            //log->SetAttribute("Exponent", DoubleValue(2.5));
            log->SetAttribute ("ReferenceDistance", DoubleValue (150));
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
            wifiChannel->SetPropagationLossModel (log);
            
        }

     // 4. Install wireless devices
      WifiHelper wifi;
      wifi.SetStandard (WIFI_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
      
      YansWifiPhyHelper phy;
      //phy.SetChannel(channel.Create());
      phy.SetChannel (wifiChannel);
      
      WifiMacHelper macSTA;
      macSTA.SetType ("ns3::StaWifiMac");
      NetDeviceContainer staDevices;
      staDevices = wifi.Install (phy, macSTA, wifiStaNodes);
      WifiMacHelper macAP;
      macAP.SetType ("ns3::ApWifiMac");
      NetDeviceContainer apDevice;
      apDevice = wifi.Install (phy, macAP, wifiApNode);
    
      
     // 5. Install TCP/IP stack & assign IP addresses
      InternetStackHelper stack;
      stack.Install (wifiApNode);
      stack.Install (wifiStaNodes);
      
      Ipv4AddressHelper address;
      address.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer ApInterface;
      ApInterface = address.Assign (apDevice);
      Ipv4InterfaceContainer StaInterface;
      StaInterface = address.Assign (staDevices);
      
    std::cout << " ====== ADRESSES ====== \n" ;
    std::cout << " STA1: " << StaInterface.GetAddress (0) << "\n";
    std::cout << " STA2: " << StaInterface.GetAddress (1) << "\n";
    std::cout << " STA3: " << StaInterface.GetAddress (2) << "\n";
    std::cout << " AP1: " << ApInterface.GetAddress (0) << "\n";
    std::cout << " AP2: " << ApInterface.GetAddress (1) << "\n";
    std::cout << " AP3: " << ApInterface.GetAddress (2) << "\n";
      std::cout << "-----------------------\n" ;
      
     // 6. Install applications: two CBR streams
      ApplicationContainer cbrApps;
      OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (0),12345));
      OnOffHelper onOffHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (1),12345));
      OnOffHelper onOffHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (2),12345));
      onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onOffHelper1.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onOffHelper2.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      
      
          // Flow 1:
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (0)));
          
          // Flow 2:
          onOffHelper1.SetAttribute ("DataRate", StringValue ("6000000bps")); //changed
          onOffHelper1.SetAttribute ("StartTime", TimeValue (Seconds (1.0))); //changed
          cbrApps.Add (onOffHelper1.Install (wifiStaNodes.Get (1)));
    
          // Flow 3:
          onOffHelper2.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper2.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper2.Install (wifiStaNodes.Get (2)));
        
          UdpEchoClientHelper client (ApInterface.GetAddress (0) , 9);
          client.SetAttribute ("MaxPackets", UintegerValue (10));
          client.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps;
    
          UdpEchoClientHelper client1 (ApInterface.GetAddress (1) , 9);
          client1.SetAttribute ("MaxPackets", UintegerValue (10));
          client1.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client1.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps1;
    
          UdpEchoClientHelper client2 (ApInterface.GetAddress (2) , 9);
          client2.SetAttribute ("MaxPackets", UintegerValue (10));
          client2.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client2.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps2;
          
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (0)));
    
      //client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      //pingApps.Add (client.Install (wifiStaNodes.Get (1)));
      
      client1.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps1.Add (client1.Install (wifiStaNodes.Get (0)));
    
      //client1.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      //pingApps1.Add (client1.Install (wifiStaNodes.Get (2)));
    
      client2.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps2.Add (client2.Install (wifiStaNodes.Get (1)));
    
      //client2.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      //pingApps2.Add (client2.Install (wifiStaNodes.Get (2)));
      
      // 7. Flow monitor
      FlowMonitorHelper flowHelper;
      Ptr<FlowMonitor> flowMonitor;
      flowMonitor = flowHelper.InstallAll();
      
      Simulator::Stop (Seconds(simulation_Time));
      Simulator::Run ();
      
      flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);
      
      flowMonitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
      
      std::cout << "-----------------------\n" ;
      
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
          if ( i->first > 2)
          {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
              std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
              std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
              std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
              std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
              std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

              std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
            }
        }
        
      // 8. Enable routing table
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Simulator::Stop (Seconds (simulation_Time));
       
     // 9. Enable sniffing
     phy.EnablePcapAll ("Homework5");
     phy.EnablePcap ("AP1", apDevice.Get (0));
     phy.EnablePcap ("AP2", apDevice.Get (1));
     phy.EnablePcap ("AP3", apDevice.Get (2));
     phy.EnablePcap ("STA1", staDevices.Get (0));
     phy.EnablePcap ("STA2", staDevices.Get (1));
     phy.EnablePcap ("STA3", staDevices.Get (2));
     
    
     
     Simulator::Run ();
     Simulator::Destroy ();
    

   }

*/

void run_more_nodes ( bool enableRTS, bool ls)
{
    // 1. Initialization
    uint32_t nSta = 5;
    uint32_t nAp = 1;
    uint64_t simulation_Time = 10;
    double distance = 150;
  
  
  if(enableRTS==false)
  {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
  }
  else
  {
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  }
  
  // 2. Creation of the Nodes
  NodeContainer wifiStaNodes;
  NodeContainer wifiApNode;
  wifiApNode.Create (nAp);
  wifiStaNodes.Create (nSta);
    // 2.1 Set Position of the Nodes
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    //AP
      positionAlloc->Add (Vector (distance, 0.0, 0.0));
    //STA
      positionAlloc->Add (Vector (0.0, 0.0, 0.0));
      positionAlloc->Add (Vector (distance*2, 0.0, 0.0));
      positionAlloc->Add (Vector ((distance/2), 129.95, 0.0));
      positionAlloc->Add (Vector ((1.5)*distance, 129.95, 0.0));
      positionAlloc->Add (Vector (distance, 259.9, 0.0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (wifiApNode);
      mobility.Install (wifiStaNodes);
    
      std::cout<<"\n ====== WIFI BASE STATION Setup: ======\n";
    
      for (NodeContainer::Iterator j = wifiStaNodes.Begin ();j != wifiStaNodes.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
       std::cout<<"\n ====== ACCESS POINT Setup: ======\n";
       for (NodeContainer::Iterator j = wifiApNode.Begin ();j != wifiApNode.End (); ++j)
        {
                  Ptr<Node> object = *j;
                  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
                  NS_ASSERT (position != 0);
                  Vector pos = position->GetPosition ();
                  std::cout <<" Position: [" << pos.x << " m, " << pos.y << " m, " << pos.z << " m]\n";
        }
        
        std::cout << "-----------------------\n" ;
      
      
      // 3. Creation and setup of the channel
        Ptr<MatrixPropagationLossModel> lossModel;
        // Create channel
        Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
        
        if(ls==true){
          Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
          lossModel->SetDefaultLoss(200);
          lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (0)->GetObject<MobilityModel> (), wifiApNode.Get (2)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (3)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (1)->GetObject<MobilityModel> (), wifiApNode.Get (1)->GetObject<MobilityModel> (), 50);
          lossModel->SetLoss (wifiStaNodes.Get (4)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (2)->GetObject<MobilityModel> (), 50);
          //lossModel->SetLoss (wifiStaNodes.Get (2)->GetObject<MobilityModel> (), wifiApNode.Get (0)->GetObject<MobilityModel> (), 50);
            
            wifiChannel->SetPropagationLossModel (lossModel);
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
        }else if(ls==false){
            Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
            //log->SetAttribute("Exponent", DoubleValue(2.5));
            log->SetAttribute ("ReferenceDistance", DoubleValue (150));
            wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
            wifiChannel->SetPropagationLossModel (log);
            
        }

     // 4. Install wireless devices
      WifiHelper wifi;
      wifi.SetStandard (WIFI_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
      
      YansWifiPhyHelper phy;
      //phy.SetChannel(channel.Create());
      phy.SetChannel (wifiChannel);
      
      WifiMacHelper macSTA;
      macSTA.SetType ("ns3::StaWifiMac");
      NetDeviceContainer staDevices;
      staDevices = wifi.Install (phy, macSTA, wifiStaNodes);
      WifiMacHelper macAP;
      macAP.SetType ("ns3::ApWifiMac");
      NetDeviceContainer apDevice;
      apDevice = wifi.Install (phy, macAP, wifiApNode);
    
      
     // 5. Install TCP/IP stack & assign IP addresses
      InternetStackHelper stack;
      stack.Install (wifiApNode);
      stack.Install (wifiStaNodes);
      
      Ipv4AddressHelper address;
      address.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer ApInterface;
      ApInterface = address.Assign (apDevice);
      Ipv4InterfaceContainer StaInterface;
      StaInterface = address.Assign (staDevices);
      
    std::cout << " ====== ADRESSES ====== \n" ;
    std::cout << " STA1: " << StaInterface.GetAddress (0) << "\n";
    std::cout << " STA2: " << StaInterface.GetAddress (1) << "\n";
    std::cout << " STA3: " << StaInterface.GetAddress (2) << "\n";
    std::cout << " STA4: " << StaInterface.GetAddress (3) << "\n";
    std::cout << " STA5: " << StaInterface.GetAddress (4) << "\n";
    std::cout << " AP1: " << ApInterface.GetAddress (0) << "\n";
    std::cout << "-----------------------\n" ;
      
     // 6. Install applications: two CBR streams
      ApplicationContainer cbrApps;
      OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (ApInterface.GetAddress (0),12345));
      onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      
      
          // Flow 1:
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (0)));
          
          // Flow 2:
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps")); //changed
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0))); //changed
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (1)));
    
          // Flow 3:
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (2)));
    
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (3)));
    
          onOffHelper.SetAttribute ("DataRate", StringValue ("6000000bps"));
          onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
          cbrApps.Add (onOffHelper.Install (wifiStaNodes.Get (4)));
        
          UdpEchoClientHelper client (ApInterface.GetAddress (0) , 9);
          client.SetAttribute ("MaxPackets", UintegerValue (10));
          client.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
          client.SetAttribute ("PacketSize", UintegerValue (1024)); //changed
          ApplicationContainer pingApps;
    
          
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (0)));
    
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (1)));
      
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (2)));
    
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (3)));
    
      client.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      pingApps.Add (client.Install (wifiStaNodes.Get (4)));
    
      //client2.SetAttribute ("StartTime", TimeValue (Seconds (0)));
      //pingApps2.Add (client2.Install (wifiStaNodes.Get (2)));
      
      // 7. Flow monitor
      FlowMonitorHelper flowHelper;
      Ptr<FlowMonitor> flowMonitor;
      flowMonitor = flowHelper.InstallAll();
      
      Simulator::Stop (Seconds(simulation_Time));
      Simulator::Run ();
      
      flowMonitor->SerializeToXmlFile("NameOfFile.xml", true, true);
      
      flowMonitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
      
      std::cout << "-----------------------\n" ;
      
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
          if ( i->first > 2)
          {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
              std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
              std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
              std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
              std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
              std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

              std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
              std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
              std::cout<<"========================================================================================\n";
              std::cout << "  Retransimissions: " << (i->second.txPackets - i->second.rxPackets) << " \n";
              std::cout<<"========================================================================================\n";
            }
        }
        
      // 8. Enable routing table
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Simulator::Stop (Seconds (simulation_Time));
       
     // 9. Enable sniffing
     phy.EnablePcapAll ("Homework5");
     phy.EnablePcap ("AP1", apDevice.Get (0));
     phy.EnablePcap ("STA1", staDevices.Get (0));
     phy.EnablePcap ("STA2", staDevices.Get (1));
     phy.EnablePcap ("STA3", staDevices.Get (2));
     phy.EnablePcap ("STA4", staDevices.Get (3));
     phy.EnablePcap ("STA5", staDevices.Get (4));
     
    
     
     Simulator::Run ();
     Simulator::Destroy ();
    

   }














   int
   main (int argc, char *argv[])
   {
     bool verbose = true;
     
     CommandLine cmd (__FILE__);
     cmd.AddValue ("verbose", "Enable logging", verbose);
     cmd.Parse (argc, argv);
     
     Time::SetResolution (Time::NS);
     
     if (verbose)
     {
     LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
     LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
     }
     /*
     std::cout << "\n------------------------------------------------\n";
     std::cout << "Hidden station experiment with RTS/CTS disabled:\n"<< std::flush;
     std::cout << "------------------------------------------------\n";
     run(false,true);
     std::cout << "------------------------------------------------\n";
     std::cout << "Hidden station experiment with RTS/CTS enabled:\n"<< std::flush;
     std::cout << "------------------------------------------------\n";
     run(true,true);
     std::cout << "\n------------------------------------------------------------------------------------------------\n";
     std::cout << "Hidden station experiment with RTS/CTS disabled and LogDistancePropagationModel:\n"<< std::flush;
     std::cout << "------------------------------------------------------------------------------------------------\n";
     run(false,false);
     std::cout << "\n------------------------------------------------------------------------------------------------\n";
     std::cout << "Hidden station experiment with RTS/CTS enabled and LogDistancePropagationModel:\n"<< std::flush;
     std::cout << "------------------------------------------------------------------------------------------------\n";
     run(true,false);
       */
    
    std::cout<<"\n================================================\n";
    std::cout<<"           EXPOSED NODE PROBLEM \n";
    std::cout<<"================================================";
    std::cout<<"\nAdding a node (AP) to match the exposed node problem...";
    std::cout << "\n------------------------------------------------\n";
    std::cout << "Hidden station experiment with RTS/CTS disabled:\n"<< std::flush;
    std::cout << "------------------------------------------------\n";
    run_exposed_problem(false);
    std::cout << "\n------------------------------------------------\n";
    std::cout << "Hidden station experiment with RTS/CTS enabled:\n"<< std::flush;
    std::cout << "------------------------------------------------\n";
    run_exposed_problem(true);
       
       /*
    std::cout<<"\n================================================\n";
    std::cout<<"           MODIFICATIONS \n";
    std::cout<<"================================================";
    std::cout<<"\nAdding nodes...";
    std::cout << "\n------------------------------------------------\n";
    std::cout << "Hidden station experiment with RTS/CTS disabled:\n"<< std::flush;
    std::cout << "------------------------------------------------\n";
    //run_more_nodes(false,true);
    std::cout << "\n------------------------------------------------\n";
    std::cout << "Hidden station experiment with RTS/CTS enabled:\n"<< std::flush;
    std::cout << "------------------------------------------------\n";
    //run_more_nodes(true,true);
       */
     return 0;
   }
