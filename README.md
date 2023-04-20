# CN_CHomeworks_2

ns3-simulation
In this project, we simulate a simple network topology of 3 nodes using NS3 3.35 YANS Simulator. Note that our system coded using IEEE 802.11 protocol.
We have three type of entity in our network:   
- Client: Client is the inital sender of packets in the system. Client sending some ranodom numbers to the master node, the first phase of mapping process.    
-  Master: Master is the first packet receiver in the system. packets are sent from the client node to the master node via UDP connection. Then the master node send these packets to the mapper nodes using TCP connection.    
- Mapper: Mapper is the node which mapping the incoming data to the corresponding character. After mapping, mapper nodes send the mapped data to the client node using UDP connection.

## Results

In this project, we measure two metrics: Throughput and End to End Delay (Latency).  
We mesure Throughput in terms of Mbits/s in ```ThroughputMonitor``` function and Delay in terms of seconds in ```AverageDelayMonitor``` function.

As we can see in the following figure, the decoded messages are correct.

---------------------------------------------- Figure 1 ----------------------------------------------

We have $ 1 + 3 \times mapper\ nodes\ count $ flows in our network.  
The first flow is the flow from the client node to the master node which is a UDP flow.  
The second flow is the flow from the master node to the mapper nodes which is a TCP flow. The third flow is the acknowledgement flow from the mapper nodes to the master node which is a TCP flow.  
The last flow is the flow from the mapper nodes to the client node which is a UDP flow.

We can see the metrics values for each flow when sending a new packet every 0.1 seconds, every 0.01 seconds, and every 0.006 seconds in the following figures.

### Packets Sending Rate: 10 Packets/Second
* Client Node to Master Node  
---------------------------------------------- Figure 2 ----------------------------------------------
* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
---------------------------------------------- Figure 3 ----------------------------------------------
* Mapper Nodes to Client Node  
---------------------------------------------- Figure 4 ----------------------------------------------

### Packets Sending Rate: 100 Packets/Second
* Client Node to Master Node  
---------------------------------------------- Figure 5 ----------------------------------------------
* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
---------------------------------------------- Figure 6 ----------------------------------------------
* Mapper Nodes to Client Node  
---------------------------------------------- Figure 7 ----------------------------------------------

### Packets Sending Rate: 167 Packets/Second
* Client Node to Master Node  
---------------------------------------------- Figure 8 ----------------------------------------------
* Master Node to Mapper Nodes (and Mapper Nodes to Master Node Because of TCP Connection)  
---------------------------------------------- Figure 9 ----------------------------------------------
* Mapper Nodes to Client Node  
---------------------------------------------- Figure 10 ----------------------------------------------

As we can see in the above figures, the throughput and latency will increase when the packets sending rate increases. We expect that the throughput and latency will increase when the packets sending rate increases. Because when the packets sending rate increases, the number of packets which are sent to the network will increase. So, the network will be more busy and the throughput will increase.

