/*
 * readanno.{cc,hh} -- filters packets out with phy errors
 * John Bicket
 *
 * Copyright (c) 2004 Massachussrcrs Institute of Technology
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
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/packet_anno.hh>
#include <click/straccum.hh>
#include "readanno.hh"

CLICK_DECLS


ReadAnno::ReadAnno()
  : Element(1, 1)
{
  MOD_INC_USE_COUNT;
}

ReadAnno::~ReadAnno()
{
  MOD_DEC_USE_COUNT;
}

int
ReadAnno::configure(Vector<String> &conf, ErrorHandler *errh)
{

    if (cp_va_parse(conf, this, errh, 
		    cpKeywords,
		    cpEnd) < 0) {
      return -1;
    }
  return 0;
}

Packet *
ReadAnno::simple_action(Packet *p_in)
{

  if (p_in) {
    memcpy(p_in->all_user_anno(), p_in->data(), Packet::USER_ANNO_SIZE);
  }
  
  return p_in;
}

CLICK_ENDDECLS


EXPORT_ELEMENT(ReadAnno)


