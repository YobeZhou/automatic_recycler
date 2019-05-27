# -*- coding: UTF-8 -*-
__author__ = 'YobeZhou'

#########################################################
#	File name: object_detection.py
#	   Author: YobeZhou
#	     Date: 2018/12/12
#########################################################

# Import packages
import os
import cv2
import numpy as np
import tensorflow as tf
import sys
import socket
import time
import threading

import gc
from multiprocessing import Process, Manager
#import pdb

# Import utilites
from utils import label_map_util
from utils import visualization_utils as vis_util

process_result = 'ok'

# 向共享缓冲栈中写入数据:
def write(stack, cam, top: int) -> None:
    """
    :param cam: 摄像头参数
    :param stack: Manager.list对象
    :param top: 缓冲栈容量
    :return: None
    """
    print('Process to write: %s' % os.getpid())

    video = cv2.VideoCapture(cam)
    ret = video.set(3,1280)
    ret = video.set(4,720)

    retval = video.get(5)

    print (cam)
    print(retval)

    while(video.isOpened()):
        ret, img = video.read()
        if ret:
            stack.append(img)
            # 每到一定容量清空一次缓冲栈
            # 利用gc库，手动清理内存垃圾，防止内存溢出
            if len(stack) >= top:
                del stack[:]
                gc.collect()
        # Press 'q' to quit
            if cv2.waitKey(1) == ord('q'):
                print ("Write process has safely exited...")
                break
    # Clean up
    video.release()
    cv2.destroyAllWindows()

# read进程的子线程，用以监听接收socket通信数据包
# 这是从 threading.Thread 继承创建一个新的子类
class myThread (threading.Thread):
    def __init__(self, threadID, newSocket, data):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.newSocket = newSocket
        self.data = data
        self.cmd = 'o'
    def run(self):
        while(True):
            if (self.cmd == 'q'):
                break
            self.data = self.newSocket.recv(4)
            #print (self.data)
        print ("Receiving data thread has safely exited...")

# 在缓冲栈中读取数据:
def read(stack) -> None:

    #DST_IP = '192.168.1.106'
    DST_IP = '10.3.141.1'
    DST_PORT = 8666
    
    # time.sleep(2)
    print('Process to read: %s' % os.getpid())
    
    # 包含正在使用的对象检测模块的目录的名称
    MODEL_NAME = 'inference_graph'

    # 抓取当前工作目录的路径
    CWD_PATH = os.getcwd()

    # 冻结检测图.pb文件的路径，其中包含用于对象检测的模型
    PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,'frozen_inference_graph.pb')

    # 标签映射文件的路径
    PATH_TO_LABELS = os.path.join(CWD_PATH,'training','labelmap.pbtxt')

    # 声明对象检测器可以识别的类数
    NUM_CLASSES = 4

    # 加载标签
    label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
    categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
    category_index = label_map_util.create_category_index(categories)

    # 将Tensorflow模型加载到内存中
    detection_graph = tf.Graph()
    with detection_graph.as_default():
        od_graph_def = tf.GraphDef()
        with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tf.import_graph_def(od_graph_def, name='')

        sess = tf.Session(graph=detection_graph)


    # 输入张量是图像
    image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')

    # 输出张量是检测框，分数和类
    # 每个框表示检测到特定对象的图像的一部分
    detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')

    # 每个分数代表每个对象的置信水平
    # 分数与结果图像一起显示在结果图像上
    detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
    detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')

    # 检测到的对象数量
    num_detections = detection_graph.get_tensor_by_name('num_detections:0')

    #建立Socket，SOCK_STREAM表示Socket类型为TCP
    print("Starting socket: TCP...")
    socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #在客户端开启心跳维护
    socket_tcp.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
    socket_tcp.connect((DST_IP, DST_PORT))
    
    # 创建新线程
    thread1 = myThread(1, socket_tcp, 'ok')
    # 启动新线程
    thread1.start()

    #socket_tcp.send(bytes("Tensorflow ok!",encoding="utf-8"))
    socket_tcp.send(b'Tensorflow ok!')
    while(True):
        if len(stack) != 0: 

            # 获取框架并扩展框架尺寸以具有形状：[1，无，无，3]
            # 即单列阵列，其中列中的每个项目具有像素RGB值
            frame = stack.pop()

            frame_expanded = np.expand_dims(frame, axis=0)
            
            # 通过以图像作为输入运行模型来执行实际检测
            (boxes, scores, classes, num) = sess.run(
                [detection_boxes, detection_scores, detection_classes, num_detections],
                feed_dict={image_tensor: frame_expanded})

            # 绘制检测结果（也称为“可视化结果”）
            # 可视化图像阵列上的框和标签
            image,tablename = vis_util.my_visualize_boxes_and_labels_on_image_array(
                frame,
                np.squeeze(boxes),
                np.squeeze(classes).astype(np.int32),
                np.squeeze(scores),
                category_index,
                use_normalized_coordinates=True,
                line_thickness=8,
                min_score_thresh=0.60)
            
            # 将结果绘制在框架上并显示出来。
            cv2.imshow('Object detector V1.0 Author:Yobe Zhou', frame)
            
            datar = thread1.data
            print (datar)
            if (datar == b'ok'):
                print (tablename)
                if tablename == "metal":
                    socket_tcp.sendall(b'metal')
                elif tablename == "plastic":
                    socket_tcp.sendall(b'plastic')
                elif tablename == "paper":
                    socket_tcp.sendall(b'paper')
                elif tablename == "peel":
                    socket_tcp.sendall(b'peel')
            # 按'q'退出
            if cv2.waitKey(1) == ord('q') :
                socket_tcp.sendall(b'exit')
                thread1.cmd = 'q'
                break
    # Clean up
    thread1.join()
    socket_tcp.close()
    print ("Read process has safely exited...")            


if __name__ == '__main__':
    
    print(tf.__version__)

    sys.path.append("..")

    # Initialize webcam feed (Raspberry)
    url = 'http://10.3.141.1:8081/'

    # 父进程创建缓冲栈，并传给各个子进程：
    q = Manager().list()
    pw = Process(target=write, args=(q, url, 100))
    pr = Process(target=read, args=(q,))

    pw.start()
    pr.start()

    # 等待进程结束
    pw.join()
    pr.join()
    print ("End of program!")
    
