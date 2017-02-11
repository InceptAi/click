# flag wifi names that should be ignored, because they come from cell phones

import sys
import os
from optparse import OptionParser

class Globals(object):
  pass

G = Globals()

Makers = ["Apple", "Nexus", "Samsung", "HTC", "LG", "Lenovo", "Sony", "Asus", "Alcatel", "Alps", "Microsoft", "Xiomi", "Huawei"]

def is_bad(name):
  for pat in 'att samsung t-mobile iphone galaxy verizon metropcs'.split():
    if name.lower().startswith(pat):
      return True
  for pat in 'iphone'.split():
    if pat in name.lower():
      return True
  return False

def setup_vendor_directory(opts):
  #Setting up vendor directory
  if not os.path.isfile(opts.vendor_directory):
    sys.stderr.write("Vendor Directory not found at %s\n" % opts.vendor_directory)
    sys.exit(-1)
  vendor_directory = {}
  f = open (opts.vendor_directory, 'r')
  for vendor_mapping in f:
    vendor_words = vendor_mapping.split('\t')
    if len(vendor_words) < 2 or '#' in vendor_words[0]:
      continue
    vendor_oui, vendor_name = vendor_words[0], vendor_words[1]
    #print "vendor_oui:%s, vendor_name:%s" % (vendor_oui, vendor_name)
    is_vendor_phone_maker = False
    for maker in Makers:
      if maker.lower() in vendor_name.lower():
        is_vendor_phone_maker = True
    if is_vendor_phone_maker:
      vendor_directory[vendor_oui] = vendor_name
  return vendor_directory

def main():
  op = OptionParser()
  op.add_option("-v", "--verbose", action="store_true", help="verbose", default=False)
  op.add_option("-d","--directory", dest="vendor_directory", default='VendorDirectory.txt', help="Vendor directory for MAC address lookup")
  op.add_option("-f", "--filewithmacs", dest="file_with_macs", help="File with macs to test", default=False)
  op.add_option("-m", "--macstotest", dest="macs_to_test", help="List of comma separated MACs to test", default=False)
  (opts, args) = op.parse_args()

  #setting up vendor directory
  vendor_directory = setup_vendor_directory(opts)
  #print vendor_directory
  #Testing the file macs
  if opts.file_with_macs:
    f = open(opts.file_with_macs, 'r')
    print "".join(mac for mac in f if vendor_directory.get(mac[:8]))

  #Testing the command line macs
  if opts.macs_to_test:
    macs = opts.macs_to_test.split(',')
    print "\n".join(mac for mac in macs if vendor_directory.get(mac[:8]))

if __name__ == '__main__':
  main()
