import re
from netaddr import IPAddress
#with open('./t', 'r') as f:
#	lines = f.readlines()
#	print len(lines)
#	for line in lines:
#		try:
#			ip = IPAddress(line.strip())
#			a.add(line.strip())
#		except Exception as e:
#			print e
#			print 'illegal address %s' % line.strip()
with open('/pitrix/log/eipctl_server.log') as f:
	while True:
		line = f.readline()
		if not line:
			break
		ip = re.findall(r'(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)', line)
		for i in ip:
			try:
				IPAddress(".".join(i))
			except Exception as e:
				print e
				print 'illegal address %s' % i
class Tree(object):
        def __init__(self, root, left=None, right=None):
		self.root = root
		self.left = left
		self.right = right
        def get_child(self):
		ch = []
		if self.left:
			ch.append(self.left)
		if self.right:
			ch.append(self.right)
		return ch

def print_the_right_most(l_t):
	if not l_t:
		return
	child_list = []
	for t in l_t:
		child_list.extend(t.get_child())
	print l_t[-1].root
	return print_the_right_most(child_lst)

