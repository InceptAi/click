#ifndef CLICK_WIRELESSMONITOR_HH
#define CLICK_WIRELESSMONITOR_HH
#include <click/element.hh>
#include <click/etheraddress.hh>
#include <click/bighashmap.hh>
#include <click/vector.hh>
#include <click/glue.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
=c

WirelessMonitor

=s Wifi

Track loss rates, packet tx times for each ethernet source.

=d
Pass each packet that you hear to PassiveStats element.

*/


class WirelessMonitor : public Element { 

public:  
  WirelessMonitor();
  ~WirelessMonitor();
  
  const char *class_name() const		{ return "WirelessMonitor"; }
  const char *port_count() const		{ return PORTS_1_1; }
  const char *processing() const    	{ return AGNOSTIC; }
  
  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const     { return true; }
  
  Packet *simple_action(Packet *);
  
  class PassiveStats *_passive_stats_table;

};

CLICK_ENDDECLS
#endif
