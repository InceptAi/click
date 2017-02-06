/*
 * passivestats.{cc,hh} -- passively collects client stats
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
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include <click/straccum.hh>
#include <clicknet/ether.h>
#include <clicknet/wifi.h>
#include <clicknet/udp.h>
#include <elements/wifi/bitrate.hh>
#include "passivestats.hh"
CLICK_DECLS

#define min(x,y)      ((x)<(y) ? (x) : (y))
#define max(x,y)      ((x)>(y) ? (x) : (y))

enum {NODS = 0, FROMDS, TODS, DSTODS}; 

PassiveStats::PassiveStats()
    : _timer(this)
{
  static unsigned char bcast_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  _bcast = EtherAddress(bcast_addr);
}

PassiveStats::~PassiveStats()
{
}

int
PassiveStats::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _stats_interval = 5;
  _only_data = true;
  if (cp_va_kparse(conf, this, errh,
           "STATS_INTERVAL", cpkP+cpkM, cpUnsigned, &_stats_interval,
           "ONLY_DATA", cpkP, cpBool, &_only_data,
           cpEnd) < 0)
    return -1;
  return 0;
}

int
PassiveStats::initialize(ErrorHandler *)
{
  _timer.initialize(this);
  _timer.schedule_now();
  return 0;
}

void
PassiveStats::run_timer(Timer *)
{
  if (_stats_interval > 0) {
    _timer.schedule_after_sec(_stats_interval);
    print_xml();
  } 
}


void 
PassiveStats::print_and_clear_stats()
{
  Timestamp now = Timestamp::now();
  StringAccum sa;

  for (PassiveStats::LIter iter = _links.begin(); iter.live(); iter++) {
    WirelessLink current_link = iter.key();
    LinkInfoAllDirections ld = iter.value();
    sa << "<ap:" << current_link.return_ap().unparse_colon();
    sa << " client:" << current_link.return_client().unparse_colon();
    sa << " bssid:" << current_link.return_bssid().unparse_colon() << ">\n";
    for (PassiveStats::DIter diter = ld.begin(); diter.live(); diter++) {
      int current_link_direction = diter.key();
      PassiveStats::LinkInfo l = diter.value();
      sa << "    <direction(1-tods, 2-fromds, 3-dstods):" << current_link_direction << ">\n";
      //if (l._total_data_pkts < 100)
        //continue;
      Timestamp age = now - l._last_received;
      int avg_signal;
      int avg_noise;
      int avg_data_pkt_size;
      int avg_pkt_duration;
      sa << "        total_pkts: " << l._total_pkts << "\n";
      sa << "        total_retx: " << l._total_retx_pkts  << "\n" ;
      sa << "        total_data_pkts: " << l._total_data_pkts  << "\n" ;
      sa << "        total_data_trans: " << l._total_data_transferred << "\n" ;
      sa << "        total_trans_dur: " << l._sum_durations << "\n" ;
      if (l._total_data_pkts) {
        avg_data_pkt_size = l._total_data_transferred / l._total_data_pkts;
        sa << "        avg_data_pkt_size: " << avg_data_pkt_size << "\n";
        avg_pkt_duration = l._sum_durations / l._total_data_pkts;
        sa << "        avg_data_pkt_duration: " << avg_pkt_duration << "\n";
      }
      if (l._total_pkts) {
        avg_signal = l._sum_signal / l._total_pkts;
        sa << "        avg_signal: " << avg_signal - 256 << "\n";
        avg_noise = l._sum_noise / l._total_pkts;
        sa << "        avg_noise: " << avg_noise << "\n";
      }
      sa << "        last_received:" << age << "\n";
    }
    sa << "\n";
  }
  //_links.clear();
  click_chatter("%s",sa.c_str());
  //return sa.take_string();
}



void 
PassiveStats::print_xml()
{
  //Timestamp now = Timestamp::now();
  StringAccum sa;
  String direction_string;
  sa << "<?xml version='1.0' standalone='yes'?>\n";
  sa << "<trace file='/tmp/viveksiphone_tap0_Jan16.pcap'>\n";

  for (PassiveStats::LIter iter = _links.begin(); iter.live(); iter++) {
    WirelessLink current_link = iter.key();
    LinkInfoAllDirections ld = iter.value();
    sa << "<link ap='" << current_link.return_ap().unparse_colon() << "'";
    sa << " client='" << current_link.return_client().unparse_colon() << "'";
    sa << " bssid='" << current_link.return_bssid().unparse_colon() << "'>\n";
    for (PassiveStats::DIter diter = ld.begin(); diter.live(); diter++) {
        int current_link_direction = diter.key();
      PassiveStats::LinkInfo l = diter.value();
      switch(current_link_direction) {
      case NODS:
        direction_string = String("NODS");
        break;
      case TODS:
        direction_string = String("CLIENT-AP");
        break;
      case FROMDS:
        direction_string = String("AP-CLIENT");
        break;
      case DSTODS:
        direction_string = String("AP-AP");
        break;
      default:
        direction_string = String("Undef");
        break;
      } 
      int avg_signal;
      int avg_noise;
      int avg_data_pkt_size;
      int avg_pkt_duration;
        
      sa << "    <stream dir='" << direction_string << "'";
      sa << " total_pkts='" << l._total_pkts << "'";
      sa << " total_retx='" << l._total_retx_pkts  << "'" ;
      sa << " total_data_pkts='" << l._total_data_pkts  << "'" ;
      sa << " total_data_bytes='" << l._total_data_transferred << "'" ;
      sa << " total_trans_time_usec='" << l._sum_durations << "'" ;
      if (l._total_data_pkts) {
        avg_data_pkt_size = l._total_data_transferred / l._total_data_pkts;
        sa << " avg_data_pkt_size='" << avg_data_pkt_size << "'";
        avg_pkt_duration = l._sum_durations / l._total_data_pkts;
        sa << " avg_data_pkt_duration_usec='" << avg_pkt_duration << "'";
      }
      if (l._total_pkts) {
        avg_signal = l._sum_signal / l._total_pkts;
        sa << " avg_signal='" << avg_signal - 256 << "'";
        avg_noise = l._sum_noise / l._total_pkts;
        sa << " avg_noise='" << avg_noise << "'";
      }
      sa << " rate='" << l.percentile_rate(10)/2 << "/" << l.percentile_rate(50)/2 << "/" << l.percentile_rate(90)/2 << "'";
      sa << " snr='" << (int)l.percentile_signal(10) - 256 << "/" << (int)l.percentile_signal(50) - 256 << "/" << (int)l.percentile_signal(90) -256 << "'";
      sa << " size='" << l.percentile_size(10) << "/" << l.percentile_size(50) << "/" << l.percentile_size(90) << "'";
      sa << ">\n";
      sa << "    </stream>\n";
    }
    sa << "</link>\n";
  }
  //_links.clear();
  click_chatter("%s",sa.c_str());
  //return sa.take_string();
}



Packet *
PassiveStats::parse_packet(Packet *p_in)
{
  EtherAddress src;
  EtherAddress dst; 
  EtherAddress ap;
  EtherAddress client; 
  EtherAddress bssid;
 
  unsigned curr_rate; 
  //unsigned curr_channel; 
  uint64_t curr_tx_end; 
  unsigned curr_tx_duration = 0;
  unsigned curr_tx_len;
  uint16_t curr_tx_seq;
  
  struct click_wifi *wh = (struct click_wifi *) p_in->data(); 
  struct click_wifi_extra *ceh = WIFI_EXTRA_ANNO(p_in); 
  int type = wh->i_fc[0] & WIFI_FC0_TYPE_MASK;
  int subtype = wh->i_fc[0] & WIFI_FC0_SUBTYPE_MASK;
  //unsigned duration = (unsigned)cpu_to_le16(wh->i_dur); 
  uint16_t tx_seq = le16_to_cpu(wh->i_seq) >> WIFI_SEQ_SEQ_SHIFT;

  WirelessLink current_link;
  int direction = NODS;
  switch (wh->i_fc[1] & WIFI_FC1_DIR_MASK) {
  case WIFI_FC1_DIR_NODS:
    dst = EtherAddress(wh->i_addr1);
    src = EtherAddress(wh->i_addr2);
    ap = EtherAddress(wh->i_addr1);
    client = EtherAddress(wh->i_addr2);
    bssid = EtherAddress(wh->i_addr3);
    direction = NODS;
    break;
  case WIFI_FC1_DIR_TODS:
    bssid = EtherAddress(wh->i_addr1);
    src = EtherAddress(wh->i_addr2);
    dst = EtherAddress(wh->i_addr3);
    ap = EtherAddress(wh->i_addr3);
    client = EtherAddress(wh->i_addr2);
    direction = TODS;
    break;
  case WIFI_FC1_DIR_FROMDS:
    dst = EtherAddress(wh->i_addr1);
    bssid = EtherAddress(wh->i_addr2);
    src = EtherAddress(wh->i_addr3);
    ap = EtherAddress(wh->i_addr3);
    client = EtherAddress(wh->i_addr1);
    direction = FROMDS;
    break;
  case WIFI_FC1_DIR_DSTODS:
    dst = EtherAddress(wh->i_addr1);
    src = EtherAddress(wh->i_addr2);
    bssid = EtherAddress(wh->i_addr3);
    ap = EtherAddress(wh->i_addr2);
    client = EtherAddress(wh->i_addr1);
    direction = DSTODS;
    break;
  default:
    //click_chatter("Vivek: this frame direction not recognized\n");
    break;
  }
  current_link = WirelessLink(ap, client, bssid);
  if (_only_data && (wh->i_fc[0] & WIFI_FC0_TYPE_MASK) != WIFI_FC0_TYPE_DATA)
    return p_in;
  
  LinkInfoAllDirections *link_info_directions = _links.findp_force(current_link);
  if (!link_info_directions) {
      click_chatter("Memory overrun, no space for adding new client information\n");
  }
  LinkInfo *link_info = link_info_directions->findp_force(direction);

  curr_rate = ceh->rate;
  curr_tx_end = p_in->timestamp_anno().usecval();
  //click_chatter("mactimestamp: %lu, %lu\n", p_in->timestamp_anno(), curr_tx_end);
  curr_tx_len = p_in->length();
  curr_tx_seq = tx_seq; 
  //curr_tx_duration = max(calc_transmit_time((int)(curr_rate), (int)curr_tx_len), duration);
  curr_tx_duration = calc_usecs_wifi_packet((int)curr_tx_len, (int)(curr_rate), 0);
 
  //update link stats
  link_info->_last_received.assign_now();
  link_info->_signal_vector.push_back(ceh->rssi);
  link_info->_sum_signal += ceh->rssi;
  link_info->_sum_noise += ceh->silence;
  link_info->_total_pkts++;
  if (wh->i_fc[1] & WIFI_FC1_RETRY) {
    link_info->_total_retx_pkts++;
  }
  link_info->_last_seq_number = (unsigned) curr_tx_seq;
  
  //Vivek : Adding detailed information
  switch (type) {
  case WIFI_FC0_TYPE_MGT:
    link_info->_total_mgt_pkts++;
    switch (subtype) {
      case WIFI_FC0_SUBTYPE_ASSOC_REQ: {
        break;
      }
      case WIFI_FC0_SUBTYPE_ASSOC_RESP: {
        break;
      }
      case WIFI_FC0_SUBTYPE_REASSOC_REQ: {
        break;
      }
      case WIFI_FC0_SUBTYPE_REASSOC_RESP: {   
      break;
      }
      case WIFI_FC0_SUBTYPE_PROBE_REQ: {
        break;
      }
      case WIFI_FC0_SUBTYPE_PROBE_RESP: {
        break;
      }
      case WIFI_FC0_SUBTYPE_BEACON: {
        break;
      }
      case WIFI_FC0_SUBTYPE_ATIM: { 
        break;
      }
      case WIFI_FC0_SUBTYPE_DISASSOC: {
        link_info->_total_disassoc_pkts++;
        break;
      }
      case WIFI_FC0_SUBTYPE_AUTH: {
        break;
      }
      case WIFI_FC0_SUBTYPE_DEAUTH: {
        link_info->_total_deauth_pkts++;
        break;
      }
      default:
        break;
    }
    break;
  case WIFI_FC0_TYPE_CTL:
    link_info->_total_ctrl_pkts++;
    break;
  case WIFI_FC0_TYPE_DATA:
    link_info->_total_data_transferred += (unsigned int) curr_tx_len;
    link_info->_total_data_pkts++;
    link_info->_sum_durations  += (unsigned int)curr_tx_duration;
    link_info->_rate_data_pkts_vector.push_back(ceh->rate);
    link_info->_size_data_pkts_vector.push_back(curr_tx_len);
    break;
  default:
    //sa << "unknown-type-" << (int) (wh->i_fc[0] & WIFI_FC0_TYPE_MASK) << " ";
  break;
  }
  return p_in;
}


enum {H_STATS, H_RESET};

static String
PassiveStats_read_param(Element *e, void *thunk)
{

  PassiveStats *td = (PassiveStats *)e;
  switch ((uintptr_t) thunk) {
  case H_STATS: {
    Timestamp now = Timestamp::now(); 
    StringAccum sa;
    for (PassiveStats::LIter iter = td->_links.begin(); iter.live(); iter++) {
    WirelessLink current_link = iter.key();
    PassiveStats::LinkInfoAllDirections ld = iter.value();
      for (PassiveStats::DIter diter = ld.begin(); diter.live(); diter++) {
        int current_link_direction = diter.key();
        PassiveStats::LinkInfo l = diter.value();
        Timestamp age = now - l._last_received;
        int avg_signal = 0;
        int avg_noise = 0;
        if (l._total_pkts) {
          avg_signal = l._sum_signal / l._total_pkts;
          avg_noise = l._sum_noise / l._total_pkts;
        }
        sa << current_link.return_ap().unparse_colon();
        sa << current_link.return_client().unparse_colon();
        sa << current_link.return_bssid().unparse_colon();
        sa << "direction(1-tods, 2-fromds, 3-dstods):" << current_link_direction << "\n";
        sa << " avg signal " << avg_signal << "\n";
        sa << " avg noise " << avg_noise << "\n";
        sa << " last_received " << age << "\n";
      }
    }
    return sa.take_string();
  }
  default:
    return String();
  }
}  



static int 
PassiveStats_write_param(const String &in_s, Element *e, void *vparam,
          ErrorHandler *)
{
  PassiveStats *f = (PassiveStats *)e;
  String s = cp_uncomment(in_s);
  switch((intptr_t)vparam) {
    case H_RESET: 
      f->_links.clear(); return 0;
  }
  return 0;
}
    
void
PassiveStats::add_handlers()
{
  add_read_handler("stats", PassiveStats_read_param, (void *) H_STATS);
  add_write_handler("reset", PassiveStats_write_param, (void *) H_RESET);
}
// generate Vector template instance
#include <click/bighashmap.cc>
#include <click/vector.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<int, PassiveStats::LinkInfo>;
template class HashMap<WirelessLink, PassiveStats::LinkInfoAllDirections>;
template class Vector<PerTxPacketStats>;
template class Vector<unsigned>;
#endif
CLICK_ENDDECLS
EXPORT_ELEMENT(PassiveStats)

