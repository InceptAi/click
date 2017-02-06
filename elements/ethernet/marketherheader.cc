/*
 * marketherheader.{cc,hh} -- element sets Ethernet Header annotation
 * Vivek Shrivastava
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
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
#include "marketherheader.hh"
#include <click/confparse.hh>
#include <clicknet/ether.h>
CLICK_DECLS

MarkEtherHeader::MarkEtherHeader()
{
}

MarkEtherHeader::~MarkEtherHeader()
{
}

int
MarkEtherHeader::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _offset = 0;
  return cp_va_kparse(conf, this, errh,
		      "OFFSET", cpkP, cpUnsigned, &_offset,
		      cpEnd);
}

Packet *
MarkEtherHeader::simple_action(Packet *p)
{
  const click_ether *ether = reinterpret_cast<const click_ether *>(p->data() + _offset);
  p->set_ether_header(ether);
  return p;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MarkEtherHeader)
ELEMENT_MT_SAFE(MarkEtherHeader)
