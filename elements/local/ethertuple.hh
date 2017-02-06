// -*- c-basic-offset: 2; related-file-name: "../../lib/ipflowid.cc" -*-
#ifndef CLICK_ETHERTUPLE_HH
#define CLICK_ETHERTUPLE_HH
#include <click/etheraddress.hh>
#include <click/hashcode.hh>
CLICK_DECLS

class EtherTuple{
public:

  EtherAddress _src;
  EtherAddress _dst;

  EtherTuple() : _src(), _dst() { }

  EtherTuple(EtherAddress src, EtherAddress dst) {
      _src = src;
      _dst = dst;
  }

  bool contains_dst(EtherAddress foo) {
    return (foo == _dst);
  }

  bool contains_src(EtherAddress foo) {
    return (foo == _src);
  }

  EtherAddress return_dst() {
    return (_dst);
  }

  EtherAddress return_src() {
    return (_src);
  }

  bool other(EtherAddress foo) { return ((_src == foo) ? _dst : _src); }

  inline size_t hashcode() const;

  inline bool
  operator==(EtherTuple other)
  {
    return (other._src == _src && other._dst == _dst);
  }

};

inline size_t EtherTuple::hashcode() const
{
    return CLICK_NAME(hashcode)(_src) + CLICK_NAME(hashcode)(_dst);
}

CLICK_ENDDECLS
#endif
