/*
 * wifiseq.{cc,hh} -- set the 802.11 sequence numbers
 * John Bicket
 *
 * Copyright (c) 2004 Massachusetts Institute of Technology
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
#include "wifiseq.hh"
#include <click/etheraddress.hh>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <clicknet/wifi.h>
#include <clicknet/llc.h>
#include <click/packet_anno.hh>
#include <elements/wifi/wirelessinfo.hh>
CLICK_DECLS


WifiSeq::WifiSeq()
  : Element(1, 1)
{
  MOD_INC_USE_COUNT;
}

WifiSeq::~WifiSeq()
{
  MOD_DEC_USE_COUNT;
}

int
WifiSeq::configure(Vector<String> &conf, ErrorHandler *errh)
{

  _debug = false;
  _seq = 0;
  _offset = 22;
  _bytes = 2;
  _shift = 4;

  if (cp_va_parse(conf, this, errh,
		  /* not required */
		  cpKeywords,
		  "DEBUG", cpBool, "Debug", &_debug,
		  "OFFSET", cpUnsigned, "offset", &_offset,
		  "BYTES", cpUnsigned, "bytes", &_bytes,
		  "SHIFT", cpUnsigned, "shift", &_shift,
		  cpEnd) < 0)
    return -1;

  if (_bytes != 2 && _bytes != 4) {
    return errh->error("BYTES must be either 2 or 4");
  }
  reset();

  return 0;
}


Packet *
WifiSeq::apply_seq(Packet *p_in) {
  WritablePacket *p = p_in ? p_in->uniqueify() : 0;

  if (p && p->length() > _offset + _bytes) {
    struct click_wifi_extra *ceh = (struct click_wifi_extra *) p_in->all_user_anno();
    ceh->flags |= WIFI_EXTRA_NO_SEQ;
    char *data = (char *)(p->data() + _offset);
    if (_bytes == 2) {
      *(u_int16_t *)data = (cpu_to_le16(_seq << _shift));
    } else {
      *(u_int32_t *)data = (cpu_to_le32(_seq << _shift));
    }
    _seq++;
  }
  return p;
}

void
WifiSeq::reset()
{
  _seq = 0;
}

Packet *
WifiSeq::pull(int port) {
  Packet *p = input(port).pull();
  return apply_seq(p);

}
void 
WifiSeq::push(int port, Packet *p) {
  p = apply_seq(p);
  output(port).push(p);
}
enum {H_DEBUG, H_SEQ, H_OFFSET, H_BYTES, H_SHIFT, H_RESET};

String 
WifiSeq::read_param(Element *e, void *thunk)
{
  WifiSeq *td = (WifiSeq *)e;
  switch ((uintptr_t) thunk) {
  case H_DEBUG:
    return String(td->_debug) + "\n";
  case H_SEQ:
    return String(td->_seq) + "\n";
  case H_OFFSET:
    return String(td->_offset) + "\n";
  case H_BYTES:
    return String(td->_bytes) + "\n";
  case H_SHIFT:
    return String(td->_shift) + "\n";
  default:
    return String();
  }
}

int 
WifiSeq::write_param(const String &in_s, Element *e, void *vparam,
		      ErrorHandler *errh)
{
  WifiSeq *f = (WifiSeq *)e;
  String s = cp_uncomment(in_s);
  switch((int)vparam) {
  case H_DEBUG: {    //debug
    bool debug;
    if (!cp_bool(s, &debug)) 
      return errh->error("debug parameter must be boolean");
    f->_debug = debug;
    break;
  }

  case H_SEQ: {    //debug
    u_int32_t seq;
    if (!cp_unsigned(s, &seq)) 
      return errh->error("seq parameter must be unsigned");
    f->_seq = seq;
    break;
  }
  case H_RESET: f->reset(); break;
  }
  return 0;
}
 
void
WifiSeq::add_handlers()
{
  add_default_handlers(true);

  add_read_handler("debug", read_param, (void *) H_DEBUG);
  add_read_handler("seq", read_param, (void *) H_SEQ);
  add_read_handler("offset", read_param, (void *) H_OFFSET);
  add_read_handler("bytes", read_param, (void *) H_BYTES);
  add_read_handler("shift", read_param, (void *) H_SHIFT);

  add_write_handler("debug", write_param, (void *) H_DEBUG);
  add_write_handler("seq", write_param, (void *) H_SEQ);
  add_write_handler("reset", write_param, (void *) H_RESET);
}
CLICK_ENDDECLS
EXPORT_ELEMENT(WifiSeq)