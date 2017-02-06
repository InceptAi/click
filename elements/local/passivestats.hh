#ifndef CLICK_PASSIVESTATS_HH
#define CLICK_PASSIVESTATS_HH
#include <click/element.hh>
#include <click/etheraddress.hh>
#include <click/bighashmap.hh>
#include <click/vector.hh>
#include <click/glue.hh>
#include <click/timer.hh>
#include "passivepacket.hh"
#include "wirelesslink.hh"
CLICK_DECLS

/*
=c

PassiveStats

=s Wifi

Track loss rates, packet tx times for each ethernet source.

=d
Accumulate RSSI, noise for each client you hear a packet from.


=h stats
Print information accumulated for each client

=h reset
Clear all information for each source

=a ExtraDecap

*/


class PassiveStats : public Element { 

public:  
  PassiveStats();
  ~PassiveStats();
  
  const char *class_name() const		{ return "PassiveStats"; }
  const char *port_count() const		{ return PORTS_0_0; }
  
  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const     { return true; }

  Packet *parse_packet(Packet *p_in);
  void add_handlers();
  void run_timer(Timer *);
  int initialize(ErrorHandler *);
  void print_and_clear_stats();
  void print_xml();
  static String read_handler(Element *e, void *);
  static int write_handler(const String &arg, Element *e,
               void *, ErrorHandler *errh);
 
 class LinkInfo {
  public:
    unsigned _total_pkts;
    unsigned _total_retx_pkts;
    unsigned _total_acked_pkts;
	unsigned _total_data_pkts;
	unsigned _total_mgt_pkts;
	unsigned _total_ctrl_pkts;
	unsigned _total_disassoc_pkts;
	unsigned _total_deauth_pkts;
	unsigned _total_data_transferred;
    unsigned _sum_rate_data_pkts;
    unsigned _sum_signal;
    unsigned _sum_noise;
    unsigned _sum_durations;
	unsigned _last_seq_number;
	unsigned _channel;
    Timestamp _last_received;
    Vector<unsigned> _signal_vector;
    Vector<unsigned> _rate_data_pkts_vector;
    Vector<unsigned> _size_data_pkts_vector;
    LinkInfo() { 
      memset(this, 0, sizeof(*this));
    }
    
  static int unsigned_compare(const void *va, const void *vb, void *) {
        unsigned a = *reinterpret_cast<const unsigned *>(va),
          b = *reinterpret_cast<const unsigned *>(vb);
        return a - b;
        //return (a < b) ? 1 : -1;
  }
  unsigned percentile_signal(unsigned percentile) {
		  if (_signal_vector.size() == 0) {
			  return 0;
		  }
		  int percentile_index = ((_signal_vector.size() * percentile) / 100) - 1;
		  if (percentile_index < 0) percentile_index = 0;
      click_qsort(_signal_vector.begin(), _signal_vector.size(), sizeof(unsigned), unsigned_compare, 0);
		  return _signal_vector[percentile_index];
  }
	unsigned percentile_rate(unsigned percentile) {
		if (_rate_data_pkts_vector.size() == 0) {
			return 0;
		}
		int percentile_index = ((_rate_data_pkts_vector.size() * percentile) / 100) - 1;
		if (percentile_index < 0) percentile_index = 0;
        click_qsort(_rate_data_pkts_vector.begin(), _rate_data_pkts_vector.size(), sizeof(unsigned), unsigned_compare, 0);
		return _rate_data_pkts_vector[percentile_index];
	}
	unsigned percentile_size(unsigned percentile) {
		if (_size_data_pkts_vector.size() == 0) {
			return 0;
		}
		int percentile_index = ((_size_data_pkts_vector.size() * percentile) / 100) - 1;
		if (percentile_index < 0) percentile_index = 0;
        click_qsort(_size_data_pkts_vector.begin(), _size_data_pkts_vector.size(), sizeof(unsigned), unsigned_compare, 0);
		return _size_data_pkts_vector[percentile_index];
	}

  };

  typedef HashMap<int, LinkInfo> LinkInfoAllDirections; 
  typedef LinkInfoAllDirections::const_iterator DIter;
  typedef HashMap<WirelessLink, LinkInfoAllDirections> LinkTable;
  typedef LinkTable::const_iterator LIter;

  LinkTable _links;
  EtherAddress _bcast;
  unsigned _stats_interval;
  Timer _timer;
  bool _only_data;
};


CLICK_ENDDECLS
#endif
