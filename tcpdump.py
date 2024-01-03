#!/usr/bin/python
import socket
from binascii import hexlify
import sys
import signal
import subprocess
from optparse import OptionParser

def host2str(host):
    packed_ip_addr = socket.inet_aton(host)
    hexStr = hexlify(packed_ip_addr)
    return hexStr

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-s", "--src", dest="src_host",
                      help="inner src host to filter")
    parser.add_option("-d", "--dst", dest="dst_host",
                      help="inner dst host to filter")
    parser.add_option("--device", dest="device", default="eth0",
                      help="device to listen on")
    options, _ = parser.parse_args()
    device = options.device

    if not options.src_host and not options.dst_host:
        parser.print_help()
        sys.exit(1)

    packet_accesor = ""
    if options.src_host:
        hexStr = host2str(options.src_host)
        packet_accesor += "ether[76:4]=" + "0x" + hexStr

    if options.dst_host:
        hexStr = host2str(options.dst_host)
        if packet_accesor:
            packet_accesor += " and " + "ether[80:4]=" + "0x" + hexStr
        else:
            packet_accesor += "ether[80:4]=" + "0x" + hexStr

    args = "-i %s -nel " % device  + packet_accesor
    args = args.split()
    cmd = ["/usr/sbin/tcpdump"] + args
    print cmd
    local_client = subprocess.Popen(cmd, shell=False, bufsize=-1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    print "subprocess pid is %s" % local_client.pid, "use ctrl + c to stop it"

    def exit_sub(sig_num, frame):
        stdout,stderr = local_client.communicate(sig_num)
        print stderr,
        print stdout,
        sys.exit(0)
    signal.signal(signal.SIGINT, exit_sub)

    while local_client.poll() is None:
        print local_client.stdout.readline(),
