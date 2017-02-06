require(package "model");

d :: FromDump(/tmp/vivek-espn-dot11.pcap, STOP true, FORCE_IP true)
-> IPClassifier(tcp)
-> a :: AggregateIPFlows
-> TCPCollector(tcpinfo.xml, SOURCE d, NOTIFIER a)
-> Discard;


DriverManager(stop);
