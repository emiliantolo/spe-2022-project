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
#include "ns3/propagation-loss-model.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-module.h"
#include "ns3/fd-net-device-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LaboratoryExample");

int main(int argc, char *argv[]) {
    bool verbose = true;
    bool rtscts = true;

    uint32_t staNum = 2;

    CommandLine cmd(__FILE__);

    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("rtscts", "Enable RTS/CTS", rtscts);
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
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(staNodes.Get(0));
    
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes.Get(0));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(staNodes.Get(1));

    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel>();
    lossModel->SetDefaultLoss(200);
    lossModel->SetLoss(apNodes.Get(0)->GetObject<MobilityModel>(), staNodes.Get(0)->GetObject<MobilityModel>(), 50);
    lossModel->SetLoss(apNodes.Get(0)->GetObject<MobilityModel>(), staNodes.Get(1)->GetObject<MobilityModel>(), 50);
    
    Ptr<YansWifiChannel> channel = CreateObject <YansWifiChannel> ();
    channel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
    channel->SetPropagationLossModel(lossModel);

    YansWifiPhyHelper phy;
    phy.SetErrorRateModel("ns3::NistErrorRateModel");
    phy.SetChannel(channel);

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    
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

    //server
    PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", serverAddress);
    ApplicationContainer serverApp = sinkHelper.Install(apNodes.Get(0));
    serverApp.Start(Seconds (1.0));
    serverApp.Stop(Seconds (20.0));

    //client
    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", serverAddress);
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    //onOffHelper.SetAttribute ("DataRate", StringValue ("54Mbps"));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (1000));
    //onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1)));
    ApplicationContainer clientApps;
    //onOffHelper.SetAttribute ("DataRate", StringValue ("3000000bps"));
    onOffHelper.SetAttribute ("DataRate", StringValue ("54Mbps"));
    //onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.1)));
    clientApps.Add(onOffHelper.Install(staNodes.Get(0)));
    //onOffHelper.SetAttribute ("DataRate", StringValue ("3001100bps"));
    //onOffHelper.SetAttribute ("DataRate", StringValue ("54Mbps"));
    //onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.1)));
    clientApps.Add(onOffHelper.Install(staNodes.Get(1)));
    clientApps.Start(Seconds(1.1));
    clientApps.Stop(Seconds(4.1));
       
    //phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    //phy.EnablePcap ("wifi-ap0", apDevices.Get(0));
    //phy.EnablePcap ("wifi-st0", staDevices.Get(0));
    //phy.EnablePcap ("wifi-st1", staDevices.Get(1));

    
    AsciiTraceHelper ascii; 
    phy.EnableAsciiAll (ascii.CreateFileStream ("wifi.tr"));
    

/*
    UdpEchoServerHelper echoServer(13);

    ApplicationContainer serverApps = echoServer.Install(apNodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    // installation of application
    ApplicationContainer clientApps = echoClient.Install(staNodes.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

  */



    Simulator::Stop(Seconds(5));  

    Simulator::Run();

/*
    std::cout<<apNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    std::cout<<staNodes.Get(0)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    std::cout<<staNodes.Get(1)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
    */

    Simulator::Destroy();
    return 0;
}
