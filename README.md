
# 前言
&emsp;&emsp;本系统由三大部分组成：PC端（数据处理部分），Raspberry端（数据传输部分），Arduino（主控模块部分）。
&emsp;&emsp;在系统运作方面分为两大阶段：
* 系统初始化阶段；
* 系统正常运作阶段。
&emsp;&emsp;系统启动并进行一系列的初始化后进入工作状态：
* 由Raspberry进行视频采集，并作为数据中转角色；
* PC端使用TensorFlow深度学习框架实现机器学习算法，对接收到的数据进行处理，收录未知的类别并将处理结果反馈给Raspberry；
* Raspberry根据反馈的结果通知Arduino控制硬件执行相应的分类操作。

<div align=center><img width="450" height="450" src="https://github.com/YobeZhou/automatic_recycler/blob/master/source/images/1-1_System-flow-chart.png"/></div>
<div align=center>图1-1 系统流程图</div>

# Arduino端
&emsp;&emsp;系统正常启动后，Arduino终端首先确定舵机当前的位置是否为设定的位置，如果不是将根据保存在EEPROM中的运行记录将舵机恢复至设定的位置，恢复位置后进入待命状态，开始等待串口接收到Raspberry发来的控制指令，当有分类指令到来时，开始阻塞执行分类操作。
# Raspberry端
&emsp;&emsp;Raspberry进行常规开机启动流程的最后，将执行四个脚本用以进行初始化：第一个脚本是建立无线局域网通信环境：驱动板载的WiFi模块使自身作为AP站点（可登录管理后台web界面对连接设备进行管理）；第二个脚本是启动与PC端进行网络通信的TCP通信(Raspberry作为服务器)以及与Arduino终端进行串口通信的python应用程序；第三个脚本是启动motion服务将视频数据传输至PC端的shell脚本；第四个脚本是建立TCP通信的服务端，并等待PC端的连接。
# PC端
&emsp;&emsp;PC端充当数据处理后台作用，其需要启动一个python应用程序，其中包括了与Raspberry建立局域网视频流数据的传输通道；作为客户端与Raspberry建立的TCP通信链路。

# 项目成品
<div align=center><img width="450" height="450" src="https://github.com/YobeZhou/automatic_recycler/blob/master/source/images/5-1_Front-view.JPG"/></div>
<div align=center>图5-1 正面图</div>

<div align=center><img width="450" height="450" src="https://github.com/YobeZhou/automatic_recycler/blob/master/source/images/5-2_back image.JPG"/></div>
<div align=center>图5-2 背面图</div>
