// mazu-nat.click

// This configuration is a firewalling NAT gateway. A version of this
// configuration was in daily use at Mazu Networks, Inc.
//
// Mazu was hooked up to the Internet via a single Ethernet connection (a
// cable modem). This configuration ran on a gateway machine hooked up
// to that cable modem via one Ethernet card. A second Ethernet card was
// hooked up to our internal network. Machines inside the internal network
// were given internal IP addresses in net 10.
//
// Here is a network diagram. Names in *starred boxes* must have some
// addresses specified in AddressInfo.
//
//     +---------+  
//    /           \                                              +-------
//   |             |       +-----------+           +-------+    /        
//   |  internal   |   ********     ********   **********  |   |         
//   |  network    |===*intern*     *extern*===*extern_ *  |===| outside 
//   |             |===*      *     *      *===*next_hop*  |===|  world  
//   |  *********  |   ********     ********   **********  |   |         
//   |  *intern_*  |       |  GATEWAY  |           | MODEM |    \        
//   |  *server *  |       +-----------+           +-------+     +-------
//    \ ********* /
//     +---------+
//
// The gateway supported the following functions:
//
// - Forwards arbitrary connections from the internal network to the outside
//   world.
// - Allows arbitrary FTP connections from the internal network to the outside
//   world. This requires application-level packet rewriting to support FTP's
//   PASV command.
// - New HTTP, HTTPS, and SSH connections from the outside world are allowed,
//   but they are forwarded to the internal machine `intern_server'.
// - All other packets from the outside world are dropped.


// ADDRESS INFORMATION

AddressInfo(
  intern 	10.0.0.1	10.0.0.0/8	00:50:ba:85:84:a9,
  extern	209.6.198.213	209.6.198.0/24	00:e0:98:09:ab:af,
  // extern	XX.XX.XX.XX	XX.XX.XX.0/24	00:e0:98:09:ab:af,
  extern_next_hop				02:00:0a:11:22:1f,
  intern_server	10.0.0.10
);


// DEVICE SETUP

elementclass GatewayDevice {
  $device |
  from :: FromDevice($device)
	-> output;
  input -> q :: Queue(1024)
	-> to :: ToDevice($device);
  ScheduleInfo(from .1, to 1);
}

// The following version of this element class sends a copy of every
// packet to ToLinuxSniffers, so that you can use tcpdump(1) to debug the
// gateway.

// elementclass GatewayDevice {
//   $device |
//   from :: FromDevice($device)
//	-> t1 :: Tee
// 	-> output;
//   input -> q :: Queue(1024)
//	-> t2 :: PullTee
// 	-> ToDevice($device);
//   t1[1] -> ToLinuxSniffers;
//   t2[1] -> ToLinuxSniffers($device);
//   ScheduleInfo(from .1, to 1);
// }

extern_dev :: GatewayDevice(extern:eth);
intern_dev :: GatewayDevice(intern:eth);

to_linux :: EtherEncap(0x0800, 1:1:1:1:1:1, intern)
	-> ToLinux;


// ARP MACHINERY

extern_arp_class, intern_arp_class
	:: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800, -);
intern_arpq :: ARPQuerier(intern);

extern_dev -> extern_arp_class;
extern_arp_class[0] -> ARPResponder(extern)	// ARP queries
	-> extern_dev;
extern_arp_class[1] -> ToLinux;			// ARP responses
extern_arp_class[3] -> Discard;

intern_dev -> intern_arp_class;
intern_arp_class[0] -> ARPResponder(intern)	// ARP queries
	-> intern_dev;
intern_arp_class[1] -> intern_arpr_t :: Tee;
	intern_arpr_t[0] -> ToLinux;
	intern_arpr_t[1] -> [1]intern_arpq;
intern_arp_class[3] -> Discard;


// REWRITERS

IPRewriterPatterns(to_outside_pat intern 50000-65535 - -,
		to_server_pat extern 50000-65535 intern_server -);

rw :: IPRewriter(// internal traffic to outside world
		 pattern to_outside_pat 0 1, 
		 // external traffic redirected to 'intern_server'
		 pattern to_server_pat 1 0,
		 // internal traffic redirected to 'intern_server'
		 pattern to_server_pat 1 1,
		 // virtual wire to output 0 if no mapping
		 nochange 0,
		 // drop if no mapping
		 drop);

tcp_rw :: TCPRewriter(// internal traffic to outside world
		pattern to_outside_pat 0 1,
		// everything else is dropped
		drop);


// OUTPUT PATH

ip_to_extern :: GetIPAddress(16) 
      -> CheckIPHeader
      -> EtherEncap(0x0800, extern:eth, extern_next_hop:eth)
      -> extern_dev;
ip_to_intern :: GetIPAddress(16) 
      -> CheckIPHeader
      -> [0]intern_arpq
      -> intern_dev;


// to outside world or gateway from inside network
rw[0] -> ip_to_extern_class :: IPClassifier(dst host intern, -);
  ip_to_extern_class[0] -> to_linux;
  ip_to_extern_class[1] -> ip_to_extern;
// to server
rw[1] -> ip_to_intern;

// tcp_rw is used only for FTP control traffic
tcp_rw[0] -> ip_to_extern;
tcp_rw[1] -> ip_to_intern;


// FILTER & REWRITE IP PACKETS FROM OUTSIDE

ip_from_extern :: IPClassifier(dst host intern,
			-);
my_ip_from_extern :: IPClassifier(dst tcp ssh,
			dst tcp www or https,
			src tcp port ftp,
			tcp or udp,
			-);

extern_arp_class[2] -> Strip(14)
  	-> CheckIPHeader
	-> ip_from_extern;
ip_from_extern[0] -> my_ip_from_extern;
  my_ip_from_extern[0] -> [1]rw; // SSH traffic (rewrite to server)
  my_ip_from_extern[1] -> [1]rw; // HTTP(S) traffic (rewrite to server)
  my_ip_from_extern[2] -> [1]tcp_rw; // FTP control traffic, rewrite w/tcp_rw
  my_ip_from_extern[3] -> [4]rw; // other TCP or UDP traffic, rewrite or drop
  my_ip_from_extern[4] -> Discard; // non TCP or UDP traffic is dropped
ip_from_extern[1] -> Discard;	// stuff for other people


// FILTER & REWRITE IP PACKETS FROM INSIDE

ip_from_intern :: IPClassifier(dst host intern,
			dst net intern,
			dst tcp port ftp,
			-);
my_ip_from_intern :: IPClassifier(dst tcp ssh,
			dst tcp www or https,
			src or dst port dns,
			dst tcp port auth,
			tcp or udp,
			-);

intern_arp_class[2] -> Strip(14)
  	-> CheckIPHeader
	-> ip_from_intern;
ip_from_intern[0] -> my_ip_from_intern; // stuff for 10.0.0.1 from inside
  my_ip_from_intern[0] -> to_linux; // SSH traffic to gw
  my_ip_from_intern[1] -> [2]rw; // HTTP(S) traffic, redirect to server instead
  my_ip_from_intern[2] -> Discard;  // DNS (no DNS allowed yet)
  my_ip_from_intern[3] -> to_linux; // auth traffic, gw will reject it
  my_ip_from_intern[4] -> [3]rw; // other TCP or UDP traffic, send to linux
                             	// but pass it thru rw in case it is the
				// returning redirect HTTP traffic from server
  my_ip_from_intern[5] -> to_linux; // non TCP or UDP traffic, to linux
ip_from_intern[1] -> to_linux;	// other net 10 stuff, like broadcasts
ip_from_intern[2] -> FTPPortMapper(tcp_rw, rw, to_outside_pat 0 1)
		-> [0]tcp_rw;	// FTP traffic for outside needs special
				// treatment
ip_from_intern[3] -> [0]rw;	// stuff for outside
