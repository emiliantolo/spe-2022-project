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

    uint32_t staNum = 2;

    CommandLine cmd(__FILE__);

    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("rtscts", "Enable RTS/CTS", rtscts);
    cmd.AddValue("datarate", "Data Rate Mbps", datarate);
    cmd.AddValue("packetsize", "Packet size", packetsize);
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
    apNodes.Create(1);


    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue(0.0),
                                    "MinY", DoubleValue(0.0),
                                    "DeltaX", DoubleValue(150.0),
                                    "DeltaY", DoubleValue(0.0),
                                    "GridWidth", UintegerValue(3),
                                    "LayoutType", StringValue("RowFirst"));
/*
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (150.0, 0.0, 0.0));
    positionAlloc->Add (Vector (300.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc);
*/


    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(staNodes.Get(0));
    mobility.Install(apNodes.Get(0));
    mobility.Install(staNodes.Get(1));


/*
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (200));
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.AddPropagationLoss ("ns3::RangePropagationLossModel"); //wireless range limited to 5 meters!

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
*/

    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel>();
    lossModel->SetDefaultLoss(200);
    lossModel->SetLoss(apNodes.Get(0)->GetObject<MobilityModel>(), staNodes.Get(0)->GetObject<MobilityModel>(), 50);
    lossModel->SetLoss(apNodes.Get(0)->GetObject<MobilityModel>(), staNodes.Get(1)->GetObject<MobilityModel>(), 50);

    
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
    Ipv4InterfaceContainer staWifiInterfaces;
    staWifiInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apWifiInterfaces;
    apWifiInterfaces = address.Assign(apDevices);

    uint16_t port = 8000;
    Address serverAddress = InetSocketAddress(apWifiInterfaces.GetAddress(0), port);
    
    double simulationTime = 8.0;
    double sendStart = 1.1;
    double sendTime = 5.0;

    //server
    PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", serverAddress);
    ApplicationContainer serverApp = sinkHelper.Install(apNodes.Get(0));
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
    clientApps.Add(onOffHelper.Install(staNodes.Get(0)));
    clientApps.Add(onOffHelper.Install(staNodes.Get(1)));
    
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap ("p/wifi-ap0", apDevices.Get(0), true);
    phy.EnablePcap ("p/wifi-st0", staDevices.Get(0), true);
    phy.EnablePcap ("p/wifi-st1", staDevices.Get(1), true);
    
    AsciiTraceHelper ascii; 
    phy.EnableAsciiAll (ascii.CreateFileStream ("wifi.tr"));

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
      
    /*
    std::cout<<apNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    std::cout<<staNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    std::cout<<staNodes.Get(1)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    */

    Simulator::Destroy();
    return 0;
}
