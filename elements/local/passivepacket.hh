#ifndef PASSIVEPACKET_HH
#define PASSIVEPACKET_HH

CLICK_DECLS

#define MAX_ROUNDS 256
#define min_time(x,y)      ((x)<(y) ? (x) : (y))
#define max_time(x,y)      ((x)>(y) ? (x) : (y))
#define PASSIVE_HEADROOM sizeof(click_ether) + sizeof(click_ip) + sizeof(click_udp)
#define CURRENT_ROUND(_round) (_round % MAX_ROUNDS)

#define debug_chatter(v,arg...) do{                             \
                            if((v)) click_chatter(arg);  \
                          }while(0)


/* Common for controller and ap */


  struct PerTxPacketStats {
    unsigned _txstart;
    unsigned _txduration;
    unsigned _txrate;
    unsigned _txlen;
	unsigned _txsuccess;
    PerTxPacketStats() {}
    PerTxPacketStats(unsigned curr_tx_start,
                     unsigned curr_tx_duration) {
      _txstart = curr_tx_start;
      _txduration = curr_tx_duration;
	  _txrate = 0;
	  _txlen = 0;
	  _txsuccess = 1;
    }
    PerTxPacketStats(unsigned curr_tx_start,
                     unsigned curr_tx_duration,
					 unsigned curr_tx_success) {
      _txstart = curr_tx_start;
      _txduration = curr_tx_duration;
	  _txsuccess = curr_tx_success;
	  _txrate = 0;
	  _txlen = 0;
    }
	PerTxPacketStats(unsigned curr_tx_rate,
                     unsigned curr_tx_start,
                     unsigned curr_tx_len,
                     unsigned curr_tx_duration) {
      _txstart = curr_tx_start;
      _txduration = curr_tx_duration;
	  _txrate = curr_tx_rate;
	  _txlen = curr_tx_len;
	  _txsuccess = 1;
    }
    int last_packet_to_test(PerTxPacketStats s) {
        if (_txstart + _txduration < s._txstart + s._txduration )
            return 1;
        else
            return 0;
    }
  };
  
  struct PerClientMetaData {
    //unsigned feedback_code;
    EtherAddress client_mac;
    EtherAddress ap_wireless_mac;
	unsigned channel;
	//unsigned mode;
    unsigned total_tx;
    unsigned total_retx;
    unsigned total_start_times;
	//Vivek adding for debugging
	unsigned total_data_transferred;
    unsigned total_data_packets;
    unsigned total_ctrl_packets;
    unsigned total_mgt_packets;
	unsigned total_disassoc_packets;
	unsigned total_deauth_packets;
  };
  
  struct SyncMetaData {
    unsigned _round;
    unsigned _aps;
  };
  
 struct WiredAckMetaData {
	IPAddress _client_ip;
    int _ip_id;
	int _rtt_ap_client;
	int _success;
    //unsigned int total_packets_acked;
	unsigned int _tries;
    //unsigned int _cwmin;
    //unsigned int _rate;
 };
  
 struct CWMetaData {
	unsigned int _cw_code;	// determines if the entry is valid
	int _interface; // 0: ath0, 1: ath1
	int _center_freq;
	int _width;
	int _is_current_epoch; // 1: current epoch/active interface , 0: later epoch

	//debug data, can be removed later
	int _gbl_slot_num;
};


CLICK_ENDDECLS

#endif //PASSIVEPACKET_HH 


  

