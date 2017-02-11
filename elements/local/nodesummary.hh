#ifndef CLICK_NODESUMMARY_HH
#define CLICK_NODESUMMARY_HH
#include <click/element.hh>
#include <click/etheraddress.hh>
#include <click/ipaddress.hh>
#include <clicknet/ip.h>
#include <clicknet/tcp.h>
#include <click/bighashmap.hh>
#include <click/glue.hh>
CLICK_DECLS

/*
=c

NodeSummary

=s Wifi

Track  each ethernet source.

=d



=h stats
Print information accumulated for each source

=h reset
Clear all information for each source

=a

*/


class NodeSummary : public Element { public:

  NodeSummary() CLICK_COLD;
  ~NodeSummary() CLICK_COLD;

  const char *class_name() const		{ return "NodeSummary"; }
  const char *port_count() const		{ return PORTS_0_0; }

  int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;
  int populate_vendors(String);
  void cleanup(CleanupStage) CLICK_COLD;
  bool can_live_reconfigure() const		{ return true; }

  Packet *parse_packet(Packet *);
  void update_node_info(EtherAddress, IPAddress, int, int);
  void add_handlers() CLICK_COLD;
  void print_xml();

  typedef HashMap<IPAddress, int> IPAddressTable;
  typedef IPAddressTable::const_iterator IPIter;

  struct NodeStats {
    int _count;
    int _count_ether_src;
    int _count_ip_src;
    int _count_tcp_src;
    int _count_ether_dst;
    int _count_ip_dst;
    int _count_tcp_dst;
    NodeStats() {
      _count = 0;
      _count_ether_src = 0;
      _count_ip_src = 0;
      _count_tcp_src = 0;
      _count_ether_dst = 0;
      _count_ip_dst = 0;
      _count_tcp_dst = 0;
    }
  };

  class NodeInfo {
  public:
    EtherAddress _eth;
    String _device_info;
    String _vendor;
    IPAddressTable _ip_list;
    NodeStats _stats;
    NodeInfo() {
    }
    NodeInfo(EtherAddress eth) {
      _eth = eth;
    }

  };
  typedef HashMap<EtherAddress, NodeInfo> NodeTable;
  typedef NodeTable::const_iterator NIter;

  typedef HashMap<String, String> VendorTable;
  typedef VendorTable::const_iterator VIter;

  NodeTable _nodes;
  String _output_xml_file;
  VendorTable _vendors;


};

CLICK_ENDDECLS
#endif
