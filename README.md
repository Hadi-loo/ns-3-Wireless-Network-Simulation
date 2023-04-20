# CN_CHomeworks_2

## ns3-simulation
In this project, we simulate a simple network topology of 3 nodes using NS3 3.35 YANS Simulator. Note that our system coded using IEEE 802.11 protocol.
We have three type of entity in our network:   
- Client: Client is the inital sender of packets in the system. Client sending some ranodom numbers to the master node, the first phase of mapping process.    
-  Master: Master is the first packet receiver in the system. packets are sent from the client node to the master node via UDP connection. Then the master node send these packets to the mapper nodes using TCP connection.    
- Mapper: Mapper is the node which mapping the incoming data to the corresponding character. After mapping, mapper nodes send the mapped data to the client node using UDP connection.


## Client
This is the first node in our network. Client is the inital sender of packets in the system to Master node over UDP connection. It also receives the mapped data from the mapper nodes over UDP connection.  
Client stores Ip address and port number of the master and itself which are used in the UDP connection.

### Constructor:
```cpp
client::client (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface , uint16_t client_port, Ipv4InterfaceContainer& staNodesClientInterface);
```

We need to override the StartApplication function to start the client application when the simulation starts.  

### StartApplication:
```cpp
void
client::StartApplication (void)
{
    master_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress sockAddr (staNodesMasterInterface.GetAddress(0), master_port);
    master_socket->Connect (sockAddr);

    mapper_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (staNodesClientInterface.GetAddress(0), client_port);
    mapper_socket->Bind (local);

    GenerateTraffic(master_socket, 0);
    mapper_socket->SetRecvCallback (MakeCallback (&client::HandleRead, this));
}
```

In this function, first we create a UDP socket to connect to the master node.  
Then we create a UDP socket to receive the mapped data from the mapper nodes.  
After that, we call the ```GenerateTraffic``` function to send the random numbers to the master node.  
Finally, we set the ```HandleRead``` function as the callback function to handle the incoming packets.

### HandleRead:
```cpp
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
        char ch = static_cast<char>(destinationHeader.GetData());
        cout << "decoded: " << ch << endl;
        cout << "=====================" << endl;
    }
}
```

In this function, we receive the mapped data from the mapper nodes and print the decoded character.



## Master
This is the second node in our network. Master is the first packet receiver in the system. packets are sent from the client node to the master node via UDP connection. Then the master node send these packets to the mapper nodes using TCP connection.  
Master stores Ip address and port number of the mapper nodes and itself which are used in UDP and TCP connections.

### Constructor:
```cpp
master::master (uint16_t master_port, Ipv4InterfaceContainer& staNodesMasterInterface, uint16_t mapper_port, Ipv4InterfaceContainer& staNodesMapperInterface) 
```

We need to override the StartApplication function to start the master application when the simulation starts.

### StartApplication:
```cpp
void
master::StartApplication (void)
{
    socket_client = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (staNodesMasterInterface.GetAddress(0), master_port);
    socket_client->Bind (local);

    for (int i = 0; i < MAPPER_NODES_COUNT; i++){
        socket_mappers[i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
        InetSocketAddress sockAddr = InetSocketAddress (staNodesMapperInterface.GetAddress(i), mapper_port + i);
        socket_mappers[i]->Connect (sockAddr); 
    }

    socket_client->SetRecvCallback (MakeCallback(&master::HandleRead , this));
}
‍‍‍‍‍```

In this function, first we create a UDP socket to receive the packets from the client node.  
Then we create TCP sockets to connect to the mapper nodes.  
Finally, we set the ```HandleRead``` function as the callback function to handle the incoming packets.


### HandleRead:
```cpp
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
```

In this function, we receive the packets from the client node and send them to the mapper nodes using TCP connection.


## Mapper


## sample.cc


## Results

In this project, we measure two metrics: Throughput and End to End Delay (Latency).  
We mesure Throughput in terms of Mbits/s in ```ThroughputMonitor``` function and Delay in terms of seconds in ```AverageDelayMonitor``` function.

As we can see in the following figure, the decoded messages are correct.  
![image](https://user-images.githubusercontent.com/83643850/233453693-7f9fb111-66fa-4d13-b6af-7b32a78d4a22.png)  


We have $1 + 3 \times mapper\ nodes\ count$ flows in our network.  
The first flow is the flow from the client node to the master node which is a UDP flow.  
The second flow is the flow from the master node to the mapper nodes which is a TCP flow. The third flow is the acknowledgement flow from the mapper nodes to the master node which is a TCP flow.  
The last flow is the flow from the mapper nodes to the client node which is a UDP flow.  

We can see the metrics values for each flow when sending a new packet every 0.1 seconds, every 0.01 seconds, and every 0.006 seconds using 3 mapper nodes in the following figures.

### Packets Sending Rate: 10 Packets/Second

* Client Node to Master Node  
![image](https://user-images.githubusercontent.com/83643850/233455235-b55db0a7-3ece-404a-9e47-d7c0bed1c522.png)


* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
![image](https://user-images.githubusercontent.com/83643850/233455281-465c0005-190f-49bf-99a4-7f51979a824f.png)
![image](https://user-images.githubusercontent.com/83643850/233454303-66b8a449-7552-40c3-bd21-db22662e756b.png)


* Mapper Nodes to Client Node  
![image](https://user-images.githubusercontent.com/83643850/233455336-8913fddd-0767-4c92-af15-da538a4494cb.png)


### Packets Sending Rate: 100 Packets/Second
* Client Node to Master Node  
---------------------------------------------- Figure 5 ----------------------------------------------
* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
---------------------------------------------- Figure 6 ----------------------------------------------
* Mapper Nodes to Client Node  
---------------------------------------------- Figure 7 ----------------------------------------------

### Packets Sending Rate: 167 Packets/Second
* Client Node to Master Node  
![image](https://user-images.githubusercontent.com/83643850/233456234-bb787c50-9256-4d7c-9814-64948403cd34.png)


* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
![image](https://user-images.githubusercontent.com/83643850/233456258-e9bb2f67-23a3-44bb-bc19-121b287a4132.png)
![image](https://user-images.githubusercontent.com/83643850/233456284-eccc12b4-a47c-4987-839f-962a794bbb86.png)


* Mapper Nodes to Client Node  
![image](https://user-images.githubusercontent.com/83643850/233456325-58905c20-0191-4fe6-862c-f7c15f79728c.png)



As we can see in the above figures, the throughput and latency will increase when the packets sending rate increases. We expect that the throughput and latency will increase when the packets sending rate increases. Because when the packets sending rate increases, the number of packets which are sent to the network will increase. So, the network will be more busy and the throughput will increase.

