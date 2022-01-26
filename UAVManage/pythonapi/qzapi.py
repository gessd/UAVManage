#coding=utf-8
import re
import sys
import os
import time
import PythonToCplusplusClass

#初始位置
def SetStartLocation(x, y):
    PythonToCplusplusClass.Fly_StartLocation(x, y);

#时间
def FlyTime(t):
    PythonToCplusplusClass.Fly_Time(t);
    
#添加航点
def FlyNavpoint(x,y,z,h,a,p,w):
    print("----waypoint");
    PythonToCplusplusClass.Fly_Waypoint(x,y,z,h,a,p,w);

def Start():
    print("---- start")

#起飞至
def Takeoff(z):
    FlyNavpoint(0,0,z,0,0,0,0);

#降落    
def Land():
    PythonToCplusplusClass.Fly_Moveto(3,0);

#飞行
def FlyTo(x,y,z):
    FlyNavpoint(x,y,z,0,0,0,0);

#以速度飞行
def FlyToSpeed(x,y,z,s):
    PythonToCplusplusClass.Fly_speedWaypoint(x,y,z,s);
    
#设置速度
def SetFlySpeed(s):
    PythonToCplusplusClass.Fly_setSpeed(s);
    
#悬停
def FlyHover(t):
    PythonToCplusplusClass.Fly_hover(t);

#旋转
def FlyRevolve(r):
    PythonToCplusplusClass.Fly_revolve(r);

#设置LED
def SetLedMode(m):
    PythonToCplusplusClass.Fly_LedMode(m);

#增量飞行
def FlyMoveto(d,s):
    PythonToCplusplusClass.Fly_MoveAddTo(d,s);