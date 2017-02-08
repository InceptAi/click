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
  void cleanup(CleanupStage) CLICK_COLD;
  bool can_live_reconfigure() const		{ return true; }

  Packet *parse_packet(Packet *);
  void update_node_info(EtherAddress, IPAddress, int, int);
  void add_handlers() CLICK_COLD;

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

  NodeTable _nodes;

};

CLICK_ENDDECLS
#endif
