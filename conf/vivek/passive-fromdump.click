passive_table::PassiveStats(STATS_INTERVAL 10, ONLY_DATA 1);

//FromDevice(wlxec086b132588)
FromDump(test-basic.pcap, STOP true)
-> prism2_decap :: Prism2Decap()
-> extra_decap :: ExtraDecap()
-> tap_decap :: RadiotapDecap()
-> Classifier(!0/80%f0) // filter out beacons
//-> PrintWifi()
-> wireless_monitor::WirelessMonitor(PASSIVE_STATS_TABLE passive_table)
-> Discard;
