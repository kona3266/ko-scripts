# --coding:utf-8--

import array
import socket
import os
import sys
import struct
import time
import select
import IN


# ICMP报文类型 => 回送请求报文
TYPE_ECHO_REQUEST = 8
CODE_ECHO_REQUEST_DEFAULT = 0

# ICMP报文类型 => 回送应答报文
TYPE_ECHO_REPLY = 0
CODE_ECHO_REPLY_DEFAULT = 0

# ICMP报文类型 => 数据报超时报文
TYPE_ICMP_OVERTIME = 11
CODE_TTL_OVERTIME = 0;

# ICMP报文类型 => 目的站不可达报文
TYPE_ICMP_UNREACHED = 3
CODE_NET_UNREACHED = 0
CODE_HOST_UNREACHED = 1
CODE_PORT_UNREACHED = 3

MAX_HOPS = 30  # 设置路由转发最大跳数为30
TIMEOUT = 5  # 如果一个请求超过5s未得到响应，则被认定为超时
TRIES = 4  # 对于每个中间站点，探测的次数设置为4
label = '*************{0}***via*****{1}**********'


def check_sum(data):
    """
    计算校验和
    """
    if len(data) % 2:  # 长度为奇数，则补字节
        data += b'\x00'
    s = sum(array.array('H', data))
    s = (s & 0xffff) + (s >> 16)  # 移位计算两次，以确保高16位为0
    s += (s >> 16)
    s = ~s  # 取反
    return socket.ntohs(s & 0xffff)  # 大小端处理


def get_host_info(host_addr):
    """"
    获取相应ip地址对应的主机信息
    """
    try:
        host_info = socket.gethostbyaddr(host_addr)
    except socket.error as e:
        display = '{0} (host name could not be determined)'.format(host_addr)
    else:
        display = '{0} ({1})'.format(host_addr, host_info[0])
    return display


def build_packet():
    """
    构建ICMP报文，首部内容如下：
    ————————————————————————————————————————
    |type (8) | code (8) | checksum (16)   |
    ————————————————————————————————————————
    |        id (16)     |  seq (16)       |
    ————————————————————————————————————————
    """
    # 先将检验和设置为0
    my_checksum = 0
    # 用进程号作标识
    my_id = os.getpid() & 0xffff
    # 序列号
    my_seq = 1

    # 打包出二进制首部
    my_header = struct.pack("bbHHh", TYPE_ECHO_REQUEST, CODE_ECHO_REQUEST_DEFAULT, my_checksum, my_id, my_seq)
    # 以当前系统时间作为报文的数据部分
    my_data = struct.pack("d", time.time())
    # 构建一个临时的数据报
    package = my_header + my_data

    # 利用原始数据报来计算真正的校验和
    my_checksum = check_sum(package)

    # 处理校验和的字节序列类型：主机序转换为网络序
    if sys.platform == 'darwin':
        my_checksum = socket.htons(my_checksum) & 0xffff
    else:
        my_checksum = socket.htons(my_checksum)

    # 重新构建出真正的数据包
    my_header = struct.pack("bbHHh", TYPE_ECHO_REQUEST, CODE_ECHO_REQUEST_DEFAULT, my_checksum, my_id, 1)
    ip_package = my_header + my_data
    return ip_package


def main(**kwargs):
    hostname = kwargs.get('ip')
    device = kwargs.get('device')
    print label.format(hostname, device)

    for ttl in xrange(1, MAX_HOPS):

        # 创建icmp原始套接字
        icmp_socket = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.getprotobyname("icmp"))
        icmp_socket.setsockopt(socket.SOL_SOCKET, IN.SO_BINDTODEVICE, device)
        icmp_socket.setsockopt(socket.IPPROTO_IP, socket.IP_TTL, struct.pack('I', ttl))
        icmp_socket.settimeout(TIMEOUT)

        # 构建报文并发送
        icmp_package = build_packet()
        try:
            icmp_socket.sendto(icmp_package, (hostname, 0))
        except socket.gaierror as e:
            print "Wrong!not a effective ip address!"
            return
        start_time = time.time()
        for i in range(0, TRIES):
            # 进入阻塞态，等待接收ICMP超时报文/应答报文
            select.select([icmp_socket], [], [], TIMEOUT)
            during_time = time.time() - start_time
            time_out = False if during_time <= TIMEOUT else True
            try_again = False
            # 计算阻塞的时间
            while not time_out:
                ip_package, ip_info = icmp_socket.recvfrom(1024)
                # 从IP数据报中取出ICMP报文的首部，位置在20：28，因为IP数据报首部长度为20
                icmp_header = ip_package[20:28]
                # 解析ICMP数据报首部各字段
                after_type, after_code, after_checksum, after_id, after_sequence = struct.unpack("bbHHh", icmp_header)
                output = ip_info[0]
                if after_type == 8:
                    try_again = True
                    # 忽略其他主机的icmp请求
                    break
                if after_type == TYPE_ICMP_UNREACHED:  # 目的不可达
                    print "Wrong!unreached net/host/port!"
                    break
                elif after_type == TYPE_ICMP_OVERTIME:  # 超时报文
                    print " %d rtt=%.0f ms %s" % (ttl, during_time * 1000, output)
                    break
                elif after_type == 0:  # 应答报文
                    print " %d rtt=%.0f ms %s" % (ttl, during_time * 1000, output)
                    print "program run over!"
                    return
                else:
                    print "return type is %d , code is %d, ip info is %s" % (after_type, after_code, ip_info)
                    print "program run wrongly"
                    return
            if time_out:
                print " %d * * * Request timed out." % ttl
            if not try_again:
                break



if __name__ == "__main__":
    args= sys.argv[1:]
    param = {'ip': '172.31.11.42', 'device': 'eth0'}
    length = len(args)
    for i in range(length-1):
        if args[i] == "--ip":
            param['ip'] = args[i+1]
        elif args[i] == "--device":
            param['device'] = args[i+1]
    main(**param)
