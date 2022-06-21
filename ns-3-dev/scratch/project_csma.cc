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
    double datarate = 1.0;
    int packetsize = 1000;

    uint32_t csmaNum = 3;

    CommandLine cmd(__FILE__);

    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("datarate", "Data Rate Mbps", datarate);
    cmd.AddValue("packetsize", "Packet size", packetsize);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    if (verbose) {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    }

    NodeContainer csmaNodes;
    csmaNodes.Create(csmaNum);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    
    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    InternetStackHelper stack;
    stack.Install(csmaNodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(csmaDevices);

    uint16_t port = 8000;
    Address serverAddress = InetSocketAddress(interfaces.GetAddress(0), port);
    
    double simulationTime = 8.0;
    double sendStart = 1.1;
    double sendTime = 5.0;

    //server
    PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", serverAddress);
    ApplicationContainer serverApp = sinkHelper.Install(csmaNodes.Get(0));
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
    clientApps.Add(onOffHelper.Install(csmaNodes.Get(1)));
    clientApps.Add(onOffHelper.Install(csmaNodes.Get(2)));
    
    csma.EnablePcap ("csma-s0", csmaDevices.Get(0));
    csma.EnablePcap ("csma-c0", csmaDevices.Get(1));
    csma.EnablePcap ("csma-c1", csmaDevices.Get(2));
    
    AsciiTraceHelper ascii; 
    csma.EnableAsciiAll (ascii.CreateFileStream ("csma.tr"));

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
