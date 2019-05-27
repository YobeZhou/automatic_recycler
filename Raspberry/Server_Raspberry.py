#!/usr/bin/env python
# -*- coding: UTF-8 -*-
__author__ = 'YobeZhou'

#########################################################
#	File name: server_pi_stepMotor.py
#	   Author: YobeZhou
#	     Date: 2019/01/21
#########################################################
import RPi.GPIO as GPIO
import time
import sys
import socket
import serial
import threading


def setup(): 
    GPIO.setwarnings(False)

    # 按物理位置编号GPIO
    GPIO.setmode(GPIO.BOARD)

# 主进程的子线程，用以监听接收socket通信数据包
# 这是从 threading.Thread 继承创建一个新的子类
class myThread (threading.Thread):
    def __init__(self, threadID, newSocket, newData):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.newSocket = newSocket
        self.newData = newData
        self.cmd = 'o'
    def run(self):
        while(True):
            self.newData = self.newSocket.recv(1024)
            if self.newData == b'exit':
                self.cmd = 'q'
                self.newSocket.close()
                break
            #print (self.newData)
        print ("Receiving data thread has safely exited...")
        
if __name__ == '__main__':
    # 自身作为服务器端
    HOST_IP = ""
    HOST_PORT = 8666
    
    # Initialize IO
    setup()
    
    # 使用9600波特率打开串口设备,1s 超时等待
    ser = serial.Serial('/dev/ttyUSB0', 9600,timeout=1);   
    
    # 作为TCP服务端，并启动服务
    host_addr = (HOST_IP, HOST_PORT)
    socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_tcp.bind(host_addr)
    socket_tcp.listen(1)

    print("TCP server start @ %s:%d!" %(HOST_IP, HOST_PORT))
    print("wait for connection...")

	# 开始接收TCP客户的连接，等待（阻塞式）连接的到来；
	# conn--套接字对象，addr--已连接客户端的地址
    conn, addr = socket_tcp.accept()
    print ("Connected by %s", addr)

	# 创建线程，用以辅助接收socket数据包
    thread1 = myThread(1, conn, 'init')
	# 启动线程
    thread1.start()
	
	# 通知Raspberry PC端就绪
    conn.send(b'ok!')
    while(True): 
        mydata = thread1.newData
        if (len(mydata) > 0):  
            print (mydata)      
            # 将字符串写入串口
            if mydata == b'metal':
                ser.write(b'1')
            elif mydata == b'plastic':
                ser.write(b'2')
            elif mydata == b'paper':
                ser.write(b'3')
            elif mydata == b'peel':
                ser.write(b'4')
            elif mydata == b'exit':
                thread1.cmd = 'q'
                break

            # 等待直到电机空闲
            while(True):
                process_resultt = ser.read()
                if process_resultt == b'1':
                    thread1.newData = ''
                    conn.send(b'ok')
                    print ("readly")
                    break
                else:
                    conn.send(b'no')
                    print ("not readly!")
                time.sleep(0.5)
    ser.close()
    conn.close()
    socket_tcp.close()
	# 等待线程结束
    thread1.join()
    print ("End of program!")
