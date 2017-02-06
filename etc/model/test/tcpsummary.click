// dump.click
// This configuration listens on the device ath0 and 
// acts like tcpdump running on an 802.11 device.
// 
// if you run
// click dump.click -h bs.scan
// when you exit it will print out a list of the access points
// that the scanner received beacons from when click exits.

require(model);
//d :: FromDump(/tmp/viveksiphone_tap0_Jan16.pcap, STOP true, FORCE_IP true)
//d :: FromDump(/tmp/foo10.pcap, STOP true, FORCE_IP true)
d :: FromTcpdump(/tmp/foo12, STOP true)
-> IPClassifier(tcp)
-> a :: AggregateIPFlows
-> TCPCollector(tcpinfo1.xml, SOURCE d, NOTIFIER a)
-> Discard;
