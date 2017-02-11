require(model)
passive_table::PassiveStats(OUTPUT_XML_FILE wireless_summary.xml, VERBOSE 0, ONLY_DATA 0, FILTER_BY_BSSID 98:FC:11:50:AF:A6);
node_summary::NodeSummary(OUTPUT_XML_FILE node_summary.xml, VENDOR_DIR /tmp/vendors.txt);

//FromDevice(wlxec086b132588)
fd::FromDump(/tmp/test1.pcap, STOP true)
-> prism2_decap :: Prism2Decap()
-> extra_decap :: ExtraDecap()
-> tap_decap :: RadiotapDecap()
-> Classifier(!0/80%f0) // filter out beacons
//-> PrintWifi()
-> wireless_monitor::WirelessMonitor(PASSIVE_STATS_TABLE passive_table)
-> WifiDecap()
-> Strip(14)
-> checkip :: CheckIPHeader()
//-> IPPrint(ip)
-> check_tcp::IPClassifier(tcp, -)
-> af :: AggregateIPFlows
-> tcol :: TCPCollector(TRACEINFO tcpmystery_summary.xml, SOURCE fd, NOTIFIER af, INTERARRIVAL false, FULLRCVWINDOW true, WINDOWPROBE true)
-> tcpmystery::TCPMystery(tcol, SEMIRTT true)
-> loss :: CalculateTCPLossEvents(TRACEINFO tcploss_summary.xml, SOURCE fd, IP_ID false, NOTIFIER af, ACKCAUSALITY true, UNDELIVERED true)
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

checkip[1]
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

check_tcp[1]
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

ProgressBar(fd.filepos, fd.filesize, BANNER './test-basic.pcap');
DriverManager(wait, write loss.clear, /*write tifd.clear,*/ stop);
