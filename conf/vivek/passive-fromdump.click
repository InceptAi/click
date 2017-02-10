//passive_table::PassiveStats(OUTPUT_XML_FILE foo10.xml, VERBOSE 1, STATS_INTERVAL 10, ONLY_DATA 1);
//passive_table::PassiveStats(OUTPUT_XML_FILE foo11.xml, VERBOSE 1, ONLY_DATA 0);
passive_table::PassiveStats(OUTPUT_XML_FILE foo11.xml, VERBOSE 1, ONLY_DATA 0, FILTER_BY_BSSID 98:FC:11:50:AF:A6);

//FromDevice(wlxec086b132588)
FromDump(test-basic.pcap, STOP true)
-> prism2_decap :: Prism2Decap()
-> extra_decap :: ExtraDecap()
-> tap_decap :: RadiotapDecap()
-> Classifier(!0/80%f0) // filter out beacons
//-> PrintWifi()
-> wireless_monitor::WirelessMonitor(PASSIVE_STATS_TABLE passive_table)
-> Strip(14
-> check_ip::CheckIPHeader()
-> node_summary::NodeSummary()
-> Discard;
