if [ "$#" -ne 2 ]; then
  echo "./start_dobby.sh trace_dir summary_dir"
  exit 1
fi

TRACEDIR=$1
SUMMARYDIR=$2

if [ ! -d "$TRACEDIR" ]; then
  # Control will enter here if $DIRECTORY doesn't exist.
  echo "$TRACEDIR does not exist. Creating"
  mkdir -p $TRACEDIR
fi

if [ ! -d "$SUMMARYDIR" ]; then
  # Control will enter here if $DIRECTORY doesn't exist.
  echo "$SUMMARYDIR does not exist. Creating"
  mkdir -p $SUMMARYDIR
fi

# killing data decyption
sudo killall -w dot11decrypt
# starting data decryption
sudo dot11decrypt -a wpa:big10:wolves999 -p big10 -o ~/Work/HomeNetworkAssistant/home_traces -t 300 -i wlxec086b132588 &
#screen -d -m sudo ./dot11decrypt -a wpa:big10:wolves999 -p big10 -o /home/vivek/Work/HomeNetworkAssistant/home_traces -t 300 -i wlxec086b132588

config="
require(model)

passive_table::PassiveStats(OUTPUT_XML_FILE /tmp/wireless.xml, VERBOSE 0, ONLY_DATA 0, FILTER_BY_BSSID 98:FC:11:50:AF:A6);
node_summary::NodeSummary(OUTPUT_XML_FILE /tmp/node.xml);

fd::FromDump(/tmp/dobby.pcap, STOP true)
-> prism2_decap :: Prism2Decap()
-> extra_decap :: ExtraDecap()
-> tap_decap :: RadiotapDecap()
-> Classifier(!0/80%f0)
-> wireless_monitor::WirelessMonitor(PASSIVE_STATS_TABLE passive_table)
-> WifiDecap()
-> Strip(14)
-> checkip :: CheckIPHeader()
-> check_tcp::IPClassifier(tcp, -)
-> af :: AggregateIPFlows
-> tcol :: TCPCollector(TRACEINFO /tmp/tcpmystery.xml, SOURCE fd, NOTIFIER af, INTERARRIVAL false, FULLRCVWINDOW true, WINDOWPROBE true)
-> tcpmystery::TCPMystery(tcol, SEMIRTT true)
-> loss :: CalculateTCPLossEvents(TRACEINFO /tmp/tcploss.xml, SOURCE fd, IP_ID false, NOTIFIER af, ACKCAUSALITY true, UNDELIVERED true)
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

checkip[1]
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

check_tcp[1]
-> NodeMonitor(NODE_SUMMARY node_summary)
-> Discard;

ProgressBar(fd.filepos, fd.filesize, BANNER '/tmp/dobby.pcap');
DriverManager(wait, write loss.clear, /*write tifd.clear,*/ stop);

"
echo "click config:" $config
echo $config > /tmp/dobby.click

# wait for new files to come up
inotifywait -m -e close_write --exclude '/\..+' --format '%w%f' "${TRACEDIR}" | while read NEWFILE
do
  echo "File ${NEWFILE} has been created or modified"
  if echo "${NEWFILE}" | grep '/' >/dev/null; then
    dir=`echo "${NEWFILE}" | sed 's/\(.*\/\)[^\/]*/\1/'`
  else
    dir='./'
  fi
  echo $dir
  justfile=`echo "$NEWFILE" | sed 's/.*\///'`
  justbase=`echo "$justfile" | sed 's/\.pcap//'`
  echo "FILE:" $justfile "DIR:" $dir "BASE:" $justbase

  # process the file
  cp ${NEWFILE} /tmp/dobby.pcap
  
  #click -e $config
  click /tmp/dobby.click
  cp /tmp/wireless.xml $SUMMARYDIR/${justbase}_wireless.xml
  cp /tmp/tcpmystery.xml $SUMMARYDIR/${justbase}_tcpmystery.xml
  cp /tmp/tcploss.xml $SUMMARYDIR/${justbase}_tcploss.xml
  cp /tmp/node.xml $SUMMARYDIR/${justbase}_node.xml
done
