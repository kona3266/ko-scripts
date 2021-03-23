f = open('line.txt', 'w+',newline='\n')
for i in range(100000000):
    f.write('this is %s line\n' % i)
f.close()