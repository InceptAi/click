require(model)
   
//f :: FromDump(/tmp/viveksiphone_tap0_Jan16.pcap, STOP true, FORCE_IP true)
//f :: FromDump(examples/sample.dump, STOP true, FORCE_IP true)
f :: FromDump(/tmp/testing_Thu_Jan_26_13:46:35_2017.pcap, STOP true, FORCE_IP true)
-> IPClassifier(tcp)
-> af :: AggregateIPFlows
-> tcol :: TCPCollector(-, SOURCE f, NOTIFIER af)
-> TCPMystery(tcol, SEMIRTT true)
-> Discard;
