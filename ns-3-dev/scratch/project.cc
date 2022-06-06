#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h" // ce ne sono diversi
#include "ns3/ssid.h"             //for ssid in wireless network
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LaboratoryExample");

int main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t staNum = 2;
    
    CommandLine cmd(__FILE__);

    cmd.AddValue("staNum", "number of wifi STA nodes", staNum);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer staNodes;
    staNodes.Create(staNum);

    NodeContainer apNodes;
    apNodes.Create(1);

    // wifi property
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetErrorRateModel ("ns3::NistErrorRateModel");
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    
    WifiMacHelper mac;
    Ssid ssid = Ssid("spe-ssid");
    
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy, mac, apNodes);
    
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, staNodes);

    MobilityHelper mobility; // mobilità dei nodi wireless

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue(0.0),
                                    "MinY", DoubleValue(0.0),
                                    "DeltaX", DoubleValue(150.0),
                                    "DeltaY", DoubleValue(0.0),
                                    "GridWidth", UintegerValue(3),
                                    "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(staNodes.Get(0));
    
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes.Get(0));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(staNodes.Get(1));

    InternetStackHelper stack;
    stack.Install(apNodes);
    stack.Install(staNodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiInterfaces;
    wifiInterfaces = address.Assign(staDevices);
    wifiInterfaces = address.Assign(apDevices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(staNodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    // installation of application
    ApplicationContainer clientApps = echoClient.Install(staNodes.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Simulator::Stop (Seconds(10.0));  

    Simulator::Run();

    std::cout<<apNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;

    std::cout<<staNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    std::cout<<staNodes.Get(1)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;

    Simulator::Destroy();
    return 0;
}
