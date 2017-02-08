passive_table::PassiveStats(OUTPUT_XML_FILE wireless_summary.xml, VERBOSE 1, ONLY_DATA 0, FILTER_BY_BSSID 98:FC:11:50:AF:A6);
node_summary::NodeSummary(OUTPUT_XML_FILE node_summary.xml);

//FromDevice(wlxec086b132588)
FromDump(test-basic.pcap, STOP true)
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
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

checkip[1]
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

