#include <cstdlib>
#include<time.h>
#include <stdio.h>
#include <string>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/error-model.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("WifiTopology");

#define MAPPER_NODES_COUNT  3

void
ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
    uint16_t i = 0;

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();

    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);

        std::cout << "Flow ID			: "<< stats->first << " ; " << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress << std::endl;
        std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
        std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
        std::cout << "Duration		: "<< (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) << std::endl;
        std::cout << "Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds () << " Seconds" << std::endl;
        std::cout << "Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) / 1024 / 1024  << " Mbps" << std::endl;
    
        i++;

        std::cout << "---------------------------------------------------------------------------" << std::endl;
    }

    Simulator::Schedule (Seconds (10),&ThroughputMonitor, fmhelper, flowMon, em);
}

void
AverageDelayMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
    uint16_t i = 0;

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
        std::cout << "Flow ID			: "<< stats->first << " ; " << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress << std::endl;
        std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
        std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
        std::cout << "Duration		: "<< (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) << std::endl;
        std::cout << "Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds () << " Seconds" << std::endl;
        std::cout << "Sum of e2e Delay: " << stats->second.delaySum.GetSeconds () << " s" << std::endl;
        std::cout << "Average of e2e Delay: " << stats->second.delaySum.GetSeconds () / stats->second.rxPackets << " s" << std::endl;
    
        i++;

        std::cout << "---------------------------------------------------------------------------" << std::endl;
    }

    Simulator::Schedule (Seconds (10),&AverageDelayMonitor, fmhelper, flowMon, em);
}

class MyHeader : public Header 
{
public:
    MyHeader ();
    virtual ~MyHeader ();
    void SetData (uint16_t data);
    uint16_t GetData (void) const;
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize (void) const;
private:
    uint16_t m_data;
};

MyHeader::MyHeader ()
{
}

MyHeader::~MyHeader ()
{
}

TypeId
MyHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MyHeader")
        .SetParent<Header> ()
        .AddConstructor<MyHeader> ()
    ;
    return tid;
}

TypeId
MyHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
MyHeader::Print (std::ostream &os) const
{
    os << "data = " << m_data << endl;
}

uint32_t
MyHeader::GetSerializedSize (void) const
{
    return 2;
}

void
MyHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU16 (m_data);
}

uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
    m_data = start.ReadNtohU16 ();

    return 2;
}

void 
MyHeader::SetData (uint16_t data)
{
    m_data = data;
}

uint16_t 
MyHeader::GetData (void) const
{
    return m_data;
}

class master : public Application
{
private:
    uint16_t master_port;
    Ipv4InterfaceContainer staNodesMasterInterface;
    uint16_t mapper_port; 
    Ipv4InterfaceContainer staNodesMapperInterface;
    Ptr<Socket> socket_client;
    Ptr<Socket> socket_mappers[MAPPER_NODES_COUNT];
    virtual void StartApplication (void);
    void HandleRead (Ptr<Socket> socket);
public:
    master (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface, uint16_t mapper_port, Ipv4InterfaceContainer& staNodesMapperInterface);
    
    virtual ~master ();
};

class client : public Application
{
private:
    uint16_t master_port;
    Ipv4InterfaceContainer staNodesMasterInterface;
    uint16_t client_port;
    Ipv4InterfaceContainer staNodesClientInterface;

    Ptr<Socket> master_socket;
    Ptr<Socket> mapper_socket;
    
    virtual void StartApplication (void);
    void HandleRead (Ptr<Socket> socket);
public:
    client (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface , uint16_t client_port, Ipv4InterfaceContainer& staNodesClientInterface);
    virtual ~client ();

};

// TODO: needs some handlers and helper methods and probably some attributes
class mapper : public Application
{
private:
    uint16_t mapper_port;
    Ipv4InterfaceContainer staNodesMapperInterface;
    uint16_t client_port;
    Ipv4InterfaceContainer staNodesClientInterface;
    int mapper_id;
    Ptr<Socket> master_socket;
    Ptr<Socket> client_socket;

    virtual void StartApplication (void);
    void HandleRead (Ptr<Socket> socket);
    void HandleAccept (Ptr<Socket> s, const Address& from);
public:
    mapper (uint16_t mapper_port, Ipv4InterfaceContainer& staNodesMapperInterface , uint16_t client_port, Ipv4InterfaceContainer& staNodesClientInterface, int mapper_id);
    virtual ~mapper ();

};

int
main (int argc, char *argv[])
{
    double error = 0.000001;
    string bandwidth = "1Mbps";
    bool verbose = true;
    double duration = 2.0;
    bool tracing = false;

    srand(time(NULL));

    CommandLine cmd (__FILE__);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

    cmd.Parse (argc,argv);

    if (verbose)
    {
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer wifiStaNodeClient;
    wifiStaNodeClient.Create (1);

    NodeContainer wifiStaNodeMaster;
    wifiStaNodeMaster.Create (1);

    // TODO:
    NodeContainer wifiStaNodeMapper;
    wifiStaNodeMapper.Create(MAPPER_NODES_COUNT);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();

    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDeviceClient;
    staDeviceClient = wifi.Install (phy, mac, wifiStaNodeClient);
    mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

    NetDeviceContainer staDeviceMaster;
    staDeviceMaster = wifi.Install (phy, mac, wifiStaNodeMaster);
    mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));

    // TODO:
    NetDeviceContainer staDeviceMapper;
    staDeviceMapper = wifi.Install (phy, mac, wifiStaNodeMapper);
    mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (error));
    phy.SetErrorRateModel("ns3::YansErrorRateModel");

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (2.0),
                                   "DeltaY", DoubleValue (4.0),
                                   "GridWidth", UintegerValue (7),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiStaNodeClient);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiStaNodeMaster);

    // TODO:
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiStaNodeMapper);

    InternetStackHelper stack;
    stack.Install (wifiStaNodeClient);
    stack.Install (wifiStaNodeMaster);

    // TODO:
    stack.Install (wifiStaNodeMapper);

    Ipv4AddressHelper address;

    Ipv4InterfaceContainer staNodeClientInterface;
    Ipv4InterfaceContainer staNodesMasterInterface;

    // TODO: 
    Ipv4InterfaceContainer staNodesMapperInterface;

    address.SetBase ("10.1.3.0", "255.255.255.0");
    staNodeClientInterface = address.Assign (staDeviceClient);
    staNodesMasterInterface = address.Assign (staDeviceMaster);

    // TODO:
    staNodesMapperInterface = address.Assign (staDeviceMapper);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t UDP_client_port = 8078;
    uint16_t UDP_master_port = 8079;
    // TODO: change port_temp to something more meaningful
    uint16_t TCP_port = 10903;


    Ptr<client> clientApp = CreateObject<client> (UDP_master_port, staNodesMasterInterface , UDP_client_port , staNodeClientInterface);
    wifiStaNodeClient.Get (0)->AddApplication (clientApp);
    clientApp->SetStartTime (Seconds (0.0));
    clientApp->SetStopTime (Seconds (duration));  

    Ptr<master> masterApp = CreateObject<master> (UDP_master_port, staNodesMasterInterface, TCP_port, staNodesMapperInterface);
    wifiStaNodeMaster.Get (0)->AddApplication (masterApp);
    masterApp->SetStartTime (Seconds (0.0));
    masterApp->SetStopTime (Seconds (duration));

    // TODO: implement same shit for mappers
    for (int i = 0; i < MAPPER_NODES_COUNT; i++) {
        Ptr<mapper> mapperApp = CreateObject<mapper> (TCP_port, staNodesMapperInterface, UDP_client_port, staNodeClientInterface , i);
        wifiStaNodeMapper.Get (i)->AddApplication (mapperApp);
        mapperApp->SetStartTime (Seconds (0.0));
        mapperApp->SetStopTime (Seconds (duration));
    }

    NS_LOG_INFO ("Run Simulation");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // ThroughputMonitor (&flowHelper, flowMonitor, error);
    // AverageDelayMonitor (&flowHelper, flowMonitor, error);

    Simulator::Stop (Seconds (duration));
    Simulator::Run ();

    return 0;
}




















client::client (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface , uint16_t client_port, Ipv4InterfaceContainer& staNodesClientInterface)
        : master_port(master_port),
          staNodesMasterInterface(staNodesMasterInterface),
          client_port(client_port),
          staNodesClientInterface(staNodesClientInterface)
{
    std::srand (time(0));
}

client::~client ()
{
}

static void GenerateTraffic (Ptr<Socket> socket, uint16_t data)
{
    Ptr<Packet> packet = new Packet();
    MyHeader m;
    m.SetData(data);

    packet->AddHeader (m);
    packet->Print (std::cout);
    socket->Send(packet);

    Simulator::Schedule (Seconds (0.1), &GenerateTraffic, socket, rand() % 26);
}

void
client::StartApplication (void)
{
    master_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress sockAddr (staNodesMasterInterface.GetAddress(0), master_port);
    master_socket->Connect (sockAddr);

    mapper_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (staNodesClientInterface.GetAddress(0), client_port);
    // cout << "master IP: ";
    // staNodesMasterInterface.GetAddress(0).Print(std::cout);
    // cout << "   master port: " << master_port << endl;
    mapper_socket->Bind (local);

    GenerateTraffic(master_socket, 0);
    mapper_socket->SetRecvCallback (MakeCallback (&client::HandleRead, this));
}

void 
client::HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;

    while ((packet = mapper_socket->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);
        destinationHeader.Print(std::cout);
    }
}



// ========================================================================================================

master::master (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface, uint16_t mapper_port, Ipv4InterfaceContainer& staNodesMapperInterface) 
        : master_port (master_port),
          staNodesMasterInterface (staNodesMasterInterface),
          mapper_port (mapper_port),
          staNodesMapperInterface (staNodesMapperInterface)
{
    std::srand (time(0));
}

master::~master ()
{
}

void
master::StartApplication (void)
{
    socket_client = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (staNodesMasterInterface.GetAddress(0), master_port);
    cout << "master IP: ";
    staNodesMasterInterface.GetAddress(0).Print(std::cout);
    cout << "   master port: " << master_port << endl;
    socket_client->Bind (local);

    
    for (int i = 0; i < MAPPER_NODES_COUNT; i++){
        socket_mappers[i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
        InetSocketAddress sockAddr = InetSocketAddress (staNodesMapperInterface.GetAddress(i), mapper_port + i);
        cout << "mapper IP: ";
        staNodesMapperInterface.GetAddress(i).Print(std::cout);
        cout << "   mapper port: " << mapper_port + i << endl;
        int result = socket_mappers[i]->Connect (sockAddr);
        cout << "Mapper connected " << i << " " << result << endl; 
    }

    socket_client->SetRecvCallback (MakeCallback(&master::HandleRead , this));

}

void 
master::HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;

    while ((packet = socket_client->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);
        Ptr<Packet> send_packet = new Packet();
        send_packet->AddHeader(destinationHeader);
        for (auto socket_mapper : socket_mappers)
            socket_mapper->Send (send_packet);
    }
}

// ==========================================================================================

mapper::mapper (uint16_t mapper_port, Ipv4InterfaceContainer& staNodesMapperInterface , uint16_t client_port, Ipv4InterfaceContainer& staNodesClientInterface , int mapper_id)
        : mapper_port (mapper_port),
          staNodesMapperInterface (staNodesMapperInterface),
          client_port (client_port),
          staNodesClientInterface (staNodesClientInterface),
          mapper_id (mapper_id)
{
    std::srand (time(0));
}

mapper::~mapper ()
{
}


void mapper::HandleAccept (Ptr<Socket> s, const Address& from)
{
    s->SetRecvCallback (MakeCallback (&mapper::HandleRead, this));
}

void
mapper::StartApplication (void)
{
    master_socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (staNodesMapperInterface.GetAddress(mapper_id), mapper_port + mapper_id);
    master_socket->Bind (local);
    master_socket->Listen();
    
    client_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress sockAddr (staNodesClientInterface.GetAddress(0), client_port);
    client_socket->Connect (sockAddr);
    
    master_socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                               MakeCallback (&mapper::HandleAccept, this));
    

}

void 
mapper::HandleRead (Ptr<Socket> socket1)
{
    Ptr<Packet> packet;
    cout << "mapper id: " << mapper_id << endl;
    while ((packet = socket1->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);
        // cout << bytes_received << endl;
        // destinationHeader.Print(std::cout);
        Ptr<Packet> send_packet = new Packet();
        send_packet->AddHeader(destinationHeader);
        // Ptr<Packet> packet = new Packet();
        
        // MyHeader m;
        // uint16_t data = mapper_id;
        // m.SetData(data);

        // send_packet->AddHeader (m);
        // send_packet->Print (std::cout);
        client_socket->Send (send_packet);
    }
}


