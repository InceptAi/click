// dump.click
// This configuration listens on the device ath0 and 
// acts like tcpdump running on an 802.11 device.
// 
// if you run
// click dump.click -h bs.scan
// when you exit it will print out a list of the access points
// that the scanner received beacons from when click exits.

require(models);
d :: FromDump(/tmp/vivek-espn-dot11.pcap, STOP true, FORCE_IP true)
-> IPClassifier(tcp)
-> a :: AggregateIPFlows
-> TCPCollector(tcpinfo.xml, SOURCE d, NOTIFIER a)
-> Discard;
