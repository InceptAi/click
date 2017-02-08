/*
 * nodesummary.{cc,hh} -- generate node summaries
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
#include <click/args.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include <click/straccum.hh>
#include <clicknet/ether.h>
#include <clicknet/wifi.h>
#include "nodesummary.hh"
CLICK_DECLS

NodeSummary::NodeSummary()
{
}

NodeSummary::~NodeSummary()
{
}

enum {SRC = 0, DST};

Packet *
NodeSummary::parse_packet(Packet *p)
{
  const click_ether *eh = (click_ether *) p->ether_header();
  const click_ip *iph = (click_ip *) p->ip_header();
  const click_tcp *tcph = (click_tcp *) p->tcp_header();
  IPAddress src_ip, dst_ip;
  EtherAddress src_ether, dst_ether;
  int src_port = 0, dst_port = 0;
  //Update ethernet info
  if (eh) {
	  src_ether = EtherAddress(eh->ether_shost);
	  dst_ether = EtherAddress(eh->ether_dhost);
    //Update associated ip addresses
    if (iph) {
      src_ip = IPAddress(iph->ip_src);
      dst_ip = IPAddress(iph->ip_dst);
      if (tcph) {
        src_port = tcph->th_sport;
        dst_port = tcph->th_dport;
      }
    }
    // Update node info
    update_node_info(src_ether, src_ip, src_port, SRC);
    update_node_info(dst_ether, dst_ip, dst_port, DST);
  }
	return p;
}

void
NodeSummary::update_node_info(EtherAddress eth_addr, IPAddress ip_addr, int port, int direction)
{
  NodeInfo *ninfo = _nodes.findp_force(eth_addr);
  if (!ninfo) {
    click_chatter("Out of memory in update_node_info\n");
  }
  ninfo->_eth = eth_addr;
  ninfo->_count++;
  if (direction == SRC) {
    ninfo->_count_ether_src++;
    if (ip_addr)
      ninfo->_count_ip_src++;
    if (port)
      ninfo->_count_tcp_src++;
  } else {
    ninfo->_count_ether_dst++;
    if (ip_addr)
      ninfo->_count_ip_dst++;
    if (port)
      ninfo->_count_tcp_dst++;
  }
  if (ip_addr)
    ninfo->_ip_list.push_back(ip_addr);
}

enum {H_STATS, H_RESET};

static String
NodeSummary_read_param(Element *e, void *thunk)
{
  NodeSummary *td = (NodeSummary *)e;
  switch ((uintptr_t) thunk) {
  case H_STATS: {
    StringAccum sa;
    for (NodeSummary::NIter iter = td->_nodes.begin(); iter.live(); iter++) {
	    NodeSummary::NodeInfo n = iter.value();
	    sa << n._eth.unparse() << " " << n._count << "\n";
    }
    return sa.take_string();
  }

  default:
    return String();
  }

}

static int
NodeSummary_write_param(const String &in_s, Element *e, void *vparam,
		      ErrorHandler *)
{
  NodeSummary *f = (NodeSummary *)e;
  String s = cp_uncomment(in_s);
  switch((intptr_t)vparam) {
  case H_RESET: f->_nodes.clear(); return 0;
  }
  return 0;
}

void
NodeSummary::add_handlers()
{
	add_read_handler("stats", NodeSummary_read_param, H_STATS);
	add_write_handler("reset", NodeSummary_write_param, H_RESET, Handler::BUTTON);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(NodeSummary)

