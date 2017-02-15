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
// Includes for getnameinfo
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define ETHER_ADDRESS_PREFIX_SIZE 8

CLICK_DECLS

NodeSummary::NodeSummary()
{
}

NodeSummary::~NodeSummary()
{
}

enum {SRC = 0, DST};

void
NodeSummary::perform_reverse_lookup(IPAddress ip_addr, int port, String *host, String *service)
{
  *host = String("");
  *service = String("");
  if (!_resolve_ip) {
    return;
  }
  struct sockaddr_in sa;
  char hostname[1000], servicename[1000];
  int name_info_ret_value;
  if (inet_pton(AF_INET, ip_addr.unparse().c_str(), &sa.sin_addr) <= 0)
  {
    return;
  }
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  if ((name_info_ret_value=getnameinfo((struct sockaddr*)&sa, sizeof sa,
    hostname, sizeof hostname, servicename, sizeof servicename, 0)) != 0) {
    //click_chatter("getnameinfo error\n");
    return;
  }
  *host = String(hostname);
  *service = String(servicename);
  return;
}

int
NodeSummary::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _output_xml_file = String("");
  String vendors_file = String("");
  _resolve_ip = false;
  if (cp_va_kparse(conf, this, errh,
        "OUTPUT_XML_FILE", cpkP, cpString, &_output_xml_file,
        "VENDOR_DIR", cpkP, cpString, &vendors_file,
        "RESOLVE_IP", cpkP, cpBool, &_resolve_ip,
        cpEnd) < 0)
    return -1;
  if (vendors_file != "" && populate_vendors(vendors_file) < 0) {
    click_chatter("Unable to read vendors file: %s, will not do reverse lookup", vendors_file.c_str());
  }
  return 0;
}

int
NodeSummary::populate_vendors(String vendors_file)
{
  FILE *fp_vendor = fopen(vendors_file.c_str(),"r");
  if(fp_vendor == 0) {
    click_chatter("cannot open file %s", vendors_file.c_str());
    return -1;
  }
  char mac[512];
  char vendor[512];
  char buf[2048];
  while(fgets(buf, sizeof(buf), fp_vendor)){
    EtherAddress macaddress;
    String buf_string = String(buf);
    if (sscanf(buf, "%s %s", mac, vendor) == 2 && mac[0] != '#') {
      String mac_string = String(mac);
      String prefix = mac_string.substring(0, ETHER_ADDRESS_PREFIX_SIZE);
      String *vendor_string = _vendors.findp_force(prefix);
      *vendor_string = String(vendor);
    } 
  }
  fclose(fp_vendor);
  return 0;
}


void
NodeSummary::cleanup(CleanupStage)
{
  print_xml();
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
  sa << "<nodes>\n";
  for (NIter iter = _nodes.begin(); iter.live(); iter++) {
	  NodeSummary::NodeInfo n = iter.value();
	  sa << "<node ether='" << n._eth.unparse_colon() << "' ";
    if (!n._vendor.empty()) 
      sa << "vendor='" << n._vendor.c_str() << "' ";
    sa << "count='" << n._stats._count << "' ";
    sa << "count_eth_src='" << n._stats._count_ether_src << "' ";
    sa << "count_eth_dst='" << n._stats._count_ether_dst << "' ";
    sa << "count_ip_src='" << n._stats._count_ip_src << "' ";
    sa << "count_ip_dst='" << n._stats._count_ip_dst << "' ";
    sa << "count_tcp_src='" << n._stats._count_tcp_src << "' ";
    sa << "count_tcp_dst='" << n._stats._count_tcp_dst << "'>\n";
    for (IPIter ip_iter = n._ip_list.begin(); ip_iter.live(); ip_iter++) {
      IPAddress ip_key = ip_iter.key();
      IPInfo ip_info = ip_iter.value();
      sa << "<ip addr='" << ip_key.unparse() << "' ";
      if (ip_info._hostname == ip_key.unparse()) {
        sa << "hostname=''>\n";
      } else {
        sa << "hostname='" << ip_info._hostname << "'>\n";
      }
      for (PIter port_iter = ip_info._porttable.begin(); port_iter.live(); port_iter++) {
        int port_key = port_iter.key();
        String servicename = port_iter.value();
        String portstring = String(port_key);
        sa << "       <port number='" << port_key << "'";
        if (portstring == servicename) {
          sa << " servicename=''";
        } else {
          sa << " servicename='" << servicename << "'";
        }
        sa << "/>\n";
      }
      sa << "</ip>\n";
    }
    sa << "</node>\n";
    sa << "\n";
  }
  sa << "</nodes>\n";
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
  String eth_substring_colon = eth_addr.unparse_colon().substring(0,ETHER_ADDRESS_PREFIX_SIZE);
  String eth_substring_dash = eth_addr.unparse().substring(0,ETHER_ADDRESS_PREFIX_SIZE);
  if (_vendors.findp(eth_substring_colon)) {
    ninfo->_vendor = *(_vendors.findp(eth_substring_colon));
  } else if (_vendors.findp(eth_substring_dash)) {
    ninfo->_vendor = *(_vendors.findp(eth_substring_dash));
  }
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
  if(ip_addr)
  {
    IPInfo *ip_info = ninfo->_ip_list.findp_force(ip_addr);
    if (!ip_info) {
      click_chatter("Out of memory in update_node_info1\n");
    } else {
      if (!ip_info->_porttable.findp(port)) { 
        String *servicename = ip_info->_porttable.findp_force(port);
        String *hostname = new String("");
        perform_reverse_lookup(ip_addr, port, hostname, servicename);
        if (*hostname != "")
          ip_info->_hostname = *hostname;
      }
    }
  }
}

enum {H_STATS, H_RESET};

static String
NodeSummary_read_param(Element *e, void *thunk)
{
  //NodeSummary *td = (NodeSummary *)e;
  switch ((uintptr_t) thunk) {
  case H_STATS:
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
template class HashMap<int, String>;
template class HashMap<String, String>;
template class HashMap<EtherAddress, NodeSummary::NodeInfo>;
#endif
CLICK_ENDDECLS
EXPORT_ELEMENT(NodeSummary)

