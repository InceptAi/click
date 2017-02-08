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

int
NodeSummary::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _output_xml_file = String("");
  if (cp_va_kparse(conf, this, errh,
        "OUTPUT_XML_FILE", cpkP, cpString, &_output_xml_file,
        cpEnd) < 0)
    return -1;
  return 0;
}

void
NodeSummary::cleanup(CleanupStage)
{
  print_xml();
  /*
  StringAccum sa;
  for (NIter iter = _nodes.begin(); iter.live(); iter++) {
	  NodeSummary::NodeInfo n = iter.value();
	  sa << "<" << n._eth.unparse_colon() << " " << n._stats._count << ">\n";
    sa << "   <IPs: ";
    for (IPIter ip_iter = n._ip_list.begin(); ip_iter.live(); ip_iter++) {
      IPAddress ip_addr = ip_iter.key();
      int ip_addr_count = ip_iter.value();
      sa << ip_addr.unparse() << '|' << ip_addr_count << "  ";
    }
    sa << ">";
    sa << "\n\n";
  }
  click_chatter("%s", sa.take_string().c_str());
  */
}

void
NodeSummary::print_xml()
{
  if (_output_xml_file == "") {
    return;
  }

  FILE *fp_xml = fopen(_output_xml_file.c_str(),"w");
  if(fp_xml == 0) {
    click_chatter("cannot open file %s", _output_xml_file.c_str());
    return;
  }

  StringAccum sa;
  sa << "<?xml version='1.0' standalone='yes'?>\n";
  for (NIter iter = _nodes.begin(); iter.live(); iter++) {
	  NodeSummary::NodeInfo n = iter.value();
	  sa << "<node ether='" << n._eth.unparse_colon() << "' ";
    sa << "count='" << n._stats._count << "' ";
    sa << "count_eth_src='" << n._stats._count_ether_src << "' ";
    sa << "count_eth_dst='" << n._stats._count_ether_dst << "' ";
    sa << "count_ip_src='" << n._stats._count_ip_src << "' ";
    sa << "count_ip_dst='" << n._stats._count_ip_dst << "' ";
    sa << "count_tcp_src='" << n._stats._count_tcp_src << "' ";
    sa << "count_tcp_dst='" << n._stats._count_tcp_dst << "'\n";
    for (IPIter ip_iter = n._ip_list.begin(); ip_iter.live(); ip_iter++) {
      IPAddress ip_addr = ip_iter.key();
      int ip_addr_count = ip_iter.value();
      sa << "<IP addr='" << ip_addr.unparse() << "' count='" << ip_addr_count << "'/>\n";
    }
    sa << "\n\n";
  }
  fprintf(fp_xml, "%s", sa.take_string().c_str());
  fclose(fp_xml);
  click_chatter("%s", sa.take_string().c_str());
}

Packet *
NodeSummary::parse_packet(Packet *p)
{
  const click_ether *eh = (click_ether *) p->ether_header();
  const click_ip *iph = (click_ip *) p->ip_header();
  const click_tcp *tcph = (click_tcp *) p->tcp_header();
  IPAddress src_ip, dst_ip;
  EtherAddress src_ether, dst_ether;
  int src_port = 0, dst_port = 0;
  if (eh) {
	  src_ether = EtherAddress(eh->ether_shost);
	  dst_ether = EtherAddress(eh->ether_dhost);
    if (iph) {
      src_ip = IPAddress(iph->ip_src);
      dst_ip = IPAddress(iph->ip_dst);
      if (tcph) {
        src_port = tcph->th_sport;
        dst_port = tcph->th_dport;
      }
    }
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
  ninfo->_stats._count++;
  if (direction == SRC) {
    ninfo->_stats._count_ether_src++;
    if (ip_addr)
      ninfo->_stats._count_ip_src++;
    if (port)
      ninfo->_stats._count_tcp_src++;
  } else {
    ninfo->_stats._count_ether_dst++;
    if (ip_addr)
      ninfo->_stats._count_ip_dst++;
    if (port)
      ninfo->_stats._count_tcp_dst++;
  }
  if (ip_addr)
  {
    int *count_ip = ninfo->_ip_list.findp_force(ip_addr);
    if (!count_ip) {
      click_chatter("Out of memory in update_node_info1\n");
    } else {
      (*count_ip)++;
    }
  }
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
	    sa << "<" << n._eth.unparse_colon() << " " << n._stats._count << ">\n";
      sa << "   <IPs: ";
      for (NodeSummary::IPIter ip_iter = n._ip_list.begin(); ip_iter.live(); ip_iter++) {
	      IPAddress ip_addr = ip_iter.key();
	      int ip_addr_count = ip_iter.value();
        sa << ip_addr.unparse() << '|' << ip_addr_count << "  ";
      }
      sa << ">";
      sa << "\n\n";
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
 
// generate Vector template instance
#include <click/bighashmap.cc>
#include <click/vector.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<IPAddress, int>;
template class HashMap<EtherAddress, NodeSummary::NodeInfo>;
#endif
CLICK_ENDDECLS
EXPORT_ELEMENT(NodeSummary)

