import functools
import time
import re
import queue as Queue
import threading
import math
from multiprocessing import Pool,Process,Queue


def timeCost(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t1 = time.time()
        r = func(*args, **kwargs)
        t2 = time.time()
        print('time cost %s',t2-t1)
        return r
    return wrapper

def handleLine(line):
    try:
        line_num = re.search(r'\d+', line).group()
    except Exception as e:
        print(e)
        print(line)
        return 0
    return line_num

@timeCost
def singleThread():
    f = open('line.txt','r')
    global count
    while True:
        line = f.readline()
        if not line:
            break
        line_num = handleLine(line)
        count += int(line_num)
    f.close()
    return

def processTask(i, queue):
    filename = "line_" + str(i) + ".txt"
    f = open(filename,'r')
    count = 0
    while True:
        line = f.readline()
        if not line:
            break
        line_num = handleLine(line)
        count += int(line_num)
    f.close()
    queue.put(count)

@timeCost
def splitFile():
    f = open('line.txt','r')
    mB = 400
    size = 1024*1024
    num = 0
    filename = "line_" + str(num//mB) + ".txt"
    fw = open(filename, 'w+',newline='\n')
    while True:
        if num % mB == 0:
            buf = f.readline()
            fw.write(buf) #把行末信息，写入上一个文件
            filename = filename = "line_" + str(num//mB) + ".txt" #更新文件名
            fw = open(filename, 'w+',newline='\n')
        buf = f.read(size)
        if buf:
            fw.write(buf)
            num += 1
        else:
            break
    return num//mB + 1


@timeCost
def multiProcess():
    num = splitFile()
    p_list = []
    queue = Queue()
    count = 0
    for i in range(num):
        p= Process(target=processTask, args=(i, queue))
        p_list.append(p)
    for i in p_list:
        i.start()
    for i in p_list:
        i.join()
    print("all subprocess done")
    while True:
        if queue.empty():
            break
        count += queue.get()
    return count
    
if __name__ == "__main__":
    count = 0
    singleThread()
    print(count)
    count = 0
    print(multiProcess())
