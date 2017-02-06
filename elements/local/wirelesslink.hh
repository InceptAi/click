// -*- c-basic-offset: 2; related-file-name: "../../lib/ipflowid.cc" -*-
#ifndef CLICK_WIRELESSLINK_HH
#define CLICK_WIRELESSLINK_HH
#include <click/etheraddress.hh>
#include <click/hashcode.hh>
CLICK_DECLS

class WirelessLink{
public:

  EtherAddress _ap;
  EtherAddress _client;
  EtherAddress _bssid;

  WirelessLink() : _ap(), _client(), _bssid() { }

  WirelessLink(EtherAddress ap, EtherAddress client, EtherAddress bssid) {
      _ap = ap;
      _client = client;
      _bssid = bssid;
  }

  bool contains_client(EtherAddress foo) {
    return (foo == _client);
  }

  bool contains_ap(EtherAddress foo) {
    return (foo == _ap);
  }

  bool contains_bssid(EtherAddress foo) {
    return (foo == _bssid);
  }

  EtherAddress return_client() {
    return (_client);
  }

  EtherAddress return_ap() {
    return (_ap);
  }
  
  EtherAddress return_bssid() {
    return (_bssid);
  }
  

  bool other(EtherAddress foo) { return ((_ap == foo) ? _client : _ap); }

  inline size_t hashcode() const;

  inline bool
  operator==(WirelessLink other)
  {
    return (other._ap == _ap && other._client == _client && other._bssid == _bssid);
  }

};

inline size_t WirelessLink::hashcode() const
{
    return CLICK_NAME(hashcode)(_ap) + CLICK_NAME(hashcode)(_client) + CLICK_NAME(hashcode)(_bssid);
}

CLICK_ENDDECLS
#endif
