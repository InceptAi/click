#ifndef CLICK_MARKETHERHEADER_HH
#define CLICK_MARKETHERHEADER_HH
#include <click/element.hh>
CLICK_DECLS

/*
 * =c
 * MarkEtherHeader([OFFSET])
 * =s ip
 * sets Ether header annotation
 * =d
 *
 * Marks packets as Ethernet packets by setting the Ethernet Header annotation. The Ethernet
 * header starts OFFSET bytes into the packet. Default OFFSET is 0.
 *
 *
 * =a CheckIPHeader, CheckIPHeader2, StripIPHeader */

class MarkEtherHeader : public Element {

  int _offset;
  
 public:
  
  MarkEtherHeader();
  ~MarkEtherHeader();
  
  const char *class_name() const		{ return "MarkEtherHeader"; }
  const char *port_count() const		{ return PORTS_1_1; }
  int configure(Vector<String> &, ErrorHandler *);

  Packet *simple_action(Packet *);
  
};

CLICK_ENDDECLS
#endif
