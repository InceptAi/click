/*
 * wirelessmonitor.{cc,hh} -- passively collects client stats
 * Vivek Shrivastava
 *
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include <click/straccum.hh>
#include <clicknet/ether.h>
#include <clicknet/wifi.h>
#include <elements/wifi/bitrate.hh>
#include <elements/local/passivestats.hh>
#include "wirelessmonitor.hh"
CLICK_DECLS

#define min(x,y)      ((x)<(y) ? (x) : (y))
#define max(x,y)      ((x)>(y) ? (x) : (y))

WirelessMonitor::WirelessMonitor()
{
}

WirelessMonitor::~WirelessMonitor()
{
}

int
WirelessMonitor::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (cp_va_kparse(conf, this, errh,
           "PASSIVE_STATS_TABLE", cpkP+cpkM, cpElement, &_passive_stats_table,
           cpEnd) < 0)
    return -1;
  if (!_passive_stats_table || _passive_stats_table->cast("PassiveStats") == 0)
    return errh->error("PassiveStats element is not provided or not a PassiveStats");
  return 0;
}

Packet *
WirelessMonitor::simple_action(Packet *p_in)
{
	_passive_stats_table->parse_packet(p_in);
	return p_in;
}
#include <click/bighashmap.cc>
#include <click/hashmap.cc>
#include <click/vector.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
#endif

CLICK_ENDDECLS
EXPORT_ELEMENT(WirelessMonitor)

