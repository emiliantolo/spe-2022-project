#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project");

int main(int argc, char *argv[]) {
    bool verbose = true;
    bool rtscts = true;
    double datarate = 1.0;
    int packetsize = 1000;
    bool hidden = true;

    uint32_t staNum = 2;
    uint32_t apNum = 2;

    CommandLine cmd(__FILE__);

    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("rtscts", "Enable RTS/CTS", rtscts);
    cmd.AddValue("datarate", "Data Rate Mbps", datarate);
    cmd.AddValue("packetsize", "Packet size", packetsize);
    cmd.AddValue("hidden", "Hidden terminal", hidden);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    if (verbose) {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    }

    UintegerValue thr = rtscts ? UintegerValue(0) : UintegerValue(10000);
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", thr);

    NodeContainer staNodes;
    staNodes.Create(staNum);

    NodeContainer apNodes;
    apNodes.Create(apNum);
    
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue(0.0),
                                    "MinY", DoubleValue(0.0),
                                    "DeltaX", DoubleValue(150.0),
                                    "DeltaY", DoubleValue(0.0),
                                    "GridWidth", UintegerValue(4),
                                    "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes.Get(0));
    mobility.Install(staNodes.Get(0));
    mobility.Install(staNodes.Get(1));
    mobility.Install(apNodes.Get(1));

    Ptr<RangePropagationLossModel> propagationLossModel = CreateObject<RangePropagationLossModel>();
    propagationLossModel->SetAttribute("MaxRange", DoubleValue(200));
    Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel>();
    lossModel->SetAttribute("ReferenceDistance", DoubleValue (150));
    lossModel->SetAttribute("ReferenceLoss", DoubleValue (50));
    if (hidden) {
        lossModel->SetNext(propagationLossModel);
    }

    Ptr<YansWifiChannel> channel = CreateObject <YansWifiChannel> ();
    channel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
    channel->SetPropagationLossModel(lossModel);

    YansWifiPhyHelper phy;
    phy.SetChannel(channel);
    
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
    
    WifiMacHelper mac;
    Ssid ssid = Ssid("spe-ssid");
    
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy, mac, apNodes);
    
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, staNodes);

    InternetStackHelper stack;
    stack.Install(apNodes);
    stack.Install(staNodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer apWifiInterfaces;
    apWifiInterfaces = address.Assign(apDevices);
    Ipv4InterfaceContainer staWifiInterfaces;
    staWifiInterfaces = address.Assign(staDevices);

    uint16_t port = 8000;
    Address server1Address = InetSocketAddress(apWifiInterfaces.GetAddress(0), port);
    Address server2Address = InetSocketAddress(apWifiInterfaces.GetAddress(1), port);

    double simulationTime = 8.0;
    double sendStart = 1.1;
    double sendTime = 5.0;

    //server
    PacketSinkHelper sinkHelper1 ("ns3::UdpSocketFactory", server1Address);
    PacketSinkHelper sinkHelper2 ("ns3::UdpSocketFactory", server2Address);

    ApplicationContainer server1App = sinkHelper1.Install(apNodes.Get(0));
    ApplicationContainer server2App = sinkHelper2.Install(apNodes.Get(1));
    
    server1App.Start(Seconds(0));
    server1App.Stop(Seconds(simulationTime));
    server2App.Start(Seconds(0));
    server2App.Stop(Seconds(simulationTime));

    //client
    OnOffHelper onOffHelper1 ("ns3::UdpSocketFactory", server1Address);
    onOffHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper1.SetAttribute ("PacketSize", UintegerValue (packetsize));
    onOffHelper1.SetAttribute ("DataRate", StringValue (std::to_string(datarate)+"Mbps"));
    OnOffHelper onOffHelper2 ("ns3::UdpSocketFactory", server2Address);
    onOffHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper2.SetAttribute ("PacketSize", UintegerValue (packetsize));
    onOffHelper2.SetAttribute ("DataRate", StringValue (std::to_string(datarate)+"Mbps"));
    
    ApplicationContainer client1App = onOffHelper1.Install(staNodes.Get(0));
    ApplicationContainer client2App = onOffHelper2.Install(staNodes.Get(1));

    client1App.Start(Seconds(sendStart));
    client1App.Stop(Seconds(sendStart + sendTime));
    client2App.Start(Seconds(sendStart));
    client2App.Stop(Seconds(sendStart + sendTime));
    
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap ("wifi-st0", staDevices.Get(0));
    phy.EnablePcap ("wifi-st1", staDevices.Get(1));
    
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();
    
      monitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / sendTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / sendTime / 1000 / 1000  << " Mbps\n";
          std::cout << "  Packet Loss Ratio: " << (i->second.txPackets - i->second.rxPackets)*100/(double)i->second.txPackets << " %\n";
      }  

    Simulator::Destroy();
    return 0;
}
