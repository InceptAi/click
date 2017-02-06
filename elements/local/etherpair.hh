// -*- c-basic-offset: 2; related-file-name: "../../lib/ipflowid.cc" -*-
#ifndef CLICK_ETHERPAIR_HH
#define CLICK_ETHERPAIR_HH
#include <click/etheraddress.hh>
#include <click/hashcode.hh>
CLICK_DECLS

class EtherPair {
public:

  EtherAddress _ap;
  EtherAddress _client;

  EtherPair() : _ap(), _client() { }

  EtherPair(EtherAddress ap, EtherAddress client) {
      _ap = ap;
      _client = client;
  }

  bool contains_client(EtherAddress foo) {
    return (foo == _client);
  }

  bool contains_ap(EtherAddress foo) {
    return (foo == _ap);
  }

  EtherAddress return_client() {
    return (_client);
  }

  EtherAddress return_ap() {
    return (_ap);
  }


  bool other(EtherAddress foo) { return ((_ap == foo) ? _client : _ap); }

  inline size_t hashcode() const;


  inline bool
  operator==(EtherPair other)
  {
    return (other._ap == _ap && other._client == _client);
  }

};

inline size_t EtherPair::hashcode() const
{
    return CLICK_NAME(hashcode)(_ap) + CLICK_NAME(hashcode)(_client);
}

CLICK_ENDDECLS
#endif
