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
    double maxrange = 75;
    int staNum = 4;

    CommandLine cmd(__FILE__);

    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("rtscts", "Enable RTS/CTS", rtscts);
    cmd.AddValue("datarate", "Data Rate Mbps", datarate);
    cmd.AddValue("packetsize", "Packet size", packetsize);
    cmd.AddValue("maxrange", "Max range", maxrange);
    cmd.AddValue("nstations", "Number of mobile stations", staNum);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    if (verbose) {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    }

    UintegerValue thr = rtscts ? UintegerValue(0) : UintegerValue(10000);
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", thr);

    NodeContainer apNodes;
    apNodes.Create(1);

    NodeContainer csmaNodes;
    csmaNodes.Add(apNodes.Get(0));
    csmaNodes.Create(1);

    NodeContainer staNodes;
    staNodes.Create(staNum);

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
    "Bounds", RectangleValue (Rectangle (-150, 150, -150, 150)),
    "Speed", StringValue ("ns3::UniformRandomVariable[Min=15.0|Max=25.0]"));
    
    mobility.Install(staNodes);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes);

    Ptr<RangePropagationLossModel> lossModel = CreateObject<RangePropagationLossModel>();
    lossModel->SetAttribute("MaxRange", DoubleValue(maxrange));
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

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    
    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);
    
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, staNodes);

    InternetStackHelper stack;
    stack.Install(apNodes);
    stack.Install(csmaNodes.Get(1));
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer staWifiInterfaces;
    staWifiInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apWifiInterfaces;
    apWifiInterfaces = address.Assign(apDevices);
    address.SetBase("10.0.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);
    
    uint16_t port = 8000;
    Address serverAddress = InetSocketAddress(csmaInterfaces.GetAddress(1), port);
    
    double simulationTime = 8.0;
    double sendStart = 1.1;
    double sendTime = 5.0;

    //server
    PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", serverAddress);
    ApplicationContainer serverApp = sinkHelper.Install(csmaNodes.Get(1));
    serverApp.Start(Seconds(0));
    serverApp.Stop(Seconds(simulationTime));

    //client
    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", serverAddress);
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetsize));
    ApplicationContainer clientApps;
    onOffHelper.SetAttribute ("DataRate", StringValue (std::to_string(datarate)+"Mbps"));
    onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (sendStart)));
    onOffHelper.SetAttribute ("StopTime", TimeValue (Seconds (sendStart + sendTime)));
    for (int i = 0; i < staNum; i++) {
        clientApps.Add(onOffHelper.Install(staNodes.Get(i)));
    }
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    //phy.EnablePcap ("wifi-ap0", apDevices.Get(0));
    
    for (int i = 0; i < staNum; i++) {
        phy.EnablePcap ("wifi-st"+std::to_string(i), staDevices.Get(i));
    }
    
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
