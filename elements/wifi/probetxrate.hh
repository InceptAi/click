#ifndef CLICK_PROBETXRATE_HH
#define CLICK_PROBETXRATE_HH
#include <click/element.hh>
#include <click/dequeue.hh>
#include <click/etheraddress.hh>
#include <click/bighashmap.hh>
#include <click/glue.hh>
CLICK_DECLS

/*
 * =c
 * 
 * ProbeTXRate([I<KEYWORDS>])
 * 
 * =s wifi
 * 
 * Probematically determine the txrate for a give ethernet dst
 * =over 8
 *
 * =item RATE_WINDOW
 * 
 * How long to remember tx packets
 *
 * =item STEPUP
 *
 * a value from 0 to 100 of what the percentage must be before
 * the rate is increased
 * 
 * =item STEPDOWN
 *
 * a value from 0 to 100. when the percentage of successful packets
 * falls below this value, the card will try the next lowest rate
 *
 * =item BEFORE_SWITCH
 *
 * how many packets must be received before you calculate the rate
 * for a give host. i.e. if you set this to 4, each rate will try
 * at least 4 packets before switching up or down.
 *
 *
 *
 * This element should be used in conjunction with SetTXRate
 * and a wifi-enabled kernel module. (like hostap or airo).
 *
 * =a
 * SetTXRate, WifiTXFeedback
 */

static inline int calc_usecs_packet(int length, int rate, int retries) {
  assert(rate);
  assert(length);
  assert(retries < 0);

  if (!rate || !length || retries < 0) {
    return 1;
  }

  int pbcc = 0;
  int t_plcp_header = 96;
  int t_slot = 20;
  int t_ack = 304;
  int t_difs = 50;
  int t_sifs = 10;
  int cw_min = 31;
  int cw_max = 1023;


  switch (rate) {
  case 2:
    /* short preamble at 1 mbit/s */
    t_plcp_header = 192;
    /* fallthrough */
  case 4:
  case 11:
  case 22:
    t_ack = 304;
    break;
  default:
    /* with 802.11g, things are at 6 mbit/s */
    t_plcp_header = 46;
    t_slot = 9;
    t_ack = 20;
    t_difs = 28;
  }
  int packet_tx_time = (2 * (t_plcp_header + (((length + pbcc) * 8))))/ rate;
  
  int cw = cw_min;
  int expected_backoff = 0;

  /* there is backoff, even for the first packet */
  for (int x = -1; x < retries; x++) {
    expected_backoff += t_slot * cw / 2;
    cw = (cw + 1) * 2;
  }

  return expected_backoff + t_difs + (retries + 1) * (
					     packet_tx_time + 
					     t_sifs + t_ack);
}

class ProbeTXRate : public Element { public:
  
  ProbeTXRate();
  ~ProbeTXRate();
  
  const char *class_name() const		{ return "ProbeTXRate"; }
  const char *processing() const		{ return "ah/a"; }
  
  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const		{ return true; }


  void push (int, Packet *);

  Packet *pull(int);

  void process_feedback(Packet *);
  void assign_rate(Packet *);
  void add_handlers();



  bool _debug;
  unsigned _packet_size_threshold;
  String print_rates();

  struct tx_result {
    struct timeval _when;
    int _rate;
    int _tries;
    bool _success;
    
    tx_result(const struct timeval &t, int rate, int tries, bool success) : 
      _when(t), 
      _rate(rate), 
      _tries(tries), 
      _success(success)
    { }
    tx_result() {}
  };

  
  struct DstInfo {
  public:

    EtherAddress _eth;
    DEQueue<tx_result> _results;

    Vector<int> _rates;
    
    
    Vector<int> _total_usecs;
    Vector<int> _total_tries;
    Vector<int> _total_success;

    DstInfo() { 
    }

    DstInfo(EtherAddress eth) { 
      _eth = eth;
    }

    int rate_index(int rate) {
      int ndx = 0;
      for (int x = 0; x < _rates.size(); x++) {
	if (rate == _rates[x]) {
	  ndx = x;
	  break;
	}
      }
      return (ndx == _rates.size()) ? -1 : ndx;
    }

    void trim(struct timeval t) {
      while (_results.size() && timercmp(&_results[0]._when, &t, <)) {
	tx_result t = _results[0];
	_results.pop_front();

	int ndx = rate_index(t._rate);

	if (ndx < 0 || ndx > _rates.size()) {
	  click_chatter("%s: ???", 
			__func__);
	  continue;
	}
	if (t._success) {
	  _total_success[ndx]--;
	}
	int usecs = calc_usecs_packet(1500, t._rate, t._tries - 1);
	_total_usecs[ndx] -= usecs;
	_total_tries[ndx] -= t._tries;
	
      }
    }


    int pick_rate() {
      int best_ndx = -1;
      int best_usecs = 0;
      bool found = false;
      if (!_rates.size()) {
	click_chatter("no rates for %s\n",
		      _eth.s().cc());
	return 2;
      }
      for (int x = 0; x < _rates.size(); x++) {
	if (_total_success[x]) {
	  int usecs = _total_usecs[x] / _total_success[x];
	  if (!found || usecs < best_usecs) {
	    best_ndx = x;
	    best_usecs = usecs;
	    found = true;
	  }
	}
      }
      
      if (random() % 23 == 0) {
	int r = random() % _rates.size();
	//click_chatter("picking random %d\n", r);
	if (r < 0 || r > _rates.size()) {
	  click_chatter("weird random rates for %s, index %d\n",
			_eth.s().cc(), r);
	  return 2;
	}
	return _rates[r];
      }
      if (best_ndx < 0) {
	int r = random() % _rates.size();
	click_chatter("no rates to pick from for %s ..random %d\n", 
		      _eth.s().cc(),
		      r);
	if (r < 0 || r > _rates.size()) {
	  click_chatter("weird random rates for %s, index %d\n",
			_eth.s().cc(), r);
	  return 2;
	}
	return _rates[r];
      }

      return _rates[best_ndx];
    }


    void add_result(struct timeval now, int rate, int retries, int success) {
      int ndx = rate_index(rate);
      if (ndx < 0 || ndx > _rates.size()){
	return;
      }
      _total_usecs[ndx] += calc_usecs_packet(1500, rate, retries);
      _total_tries[ndx] += retries + 1;
      if (success) {
	_total_success[ndx]++;
      }
      _results.push_back(tx_result(now, rate, retries + 1, success));
    }
  };
  typedef HashMap<EtherAddress, DstInfo> NeighborTable;
  typedef NeighborTable::const_iterator NIter;

  class NeighborTable _neighbors;
  EtherAddress _bcast;
  int _rate_window_ms;
  struct timeval _rate_window;

  class AvailableRates *_rtable;

};

CLICK_ENDDECLS
#endif