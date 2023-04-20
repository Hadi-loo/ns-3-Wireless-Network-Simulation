# CN_CHomeworks_2

ns3-simulation
In this project, we simulate a simple network topology of 3 nodes using NS3 3.35 YANS Simulator. Note that our system coded using IEEE 802.11 protocol.
We have three type of entity in our network:   
- Client: Client is the inital sender of packets in the system. Client sending some ranodom numbers to the master node, the first phase of mapping process.    
-  Master: Master is the first packet receiver in the system. packets are sent from the client node to the master node via UDP connection. Then the master node send these packets to the mapper nodes using TCP connection.    
- Mapper: Mapper is the node which mapping the incoming data to the corresponding character. After mapping, mapper nodes send the mapped data to the client node using UDP connection.


