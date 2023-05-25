def FlyTakeoff(height, second):
"""
  控制无人机起飞，只有起飞后才能执行后续动作
  :param height: 起飞高度(厘米)，正整数
  :param second: 起飞耗时(秒)，正整数
"""

def FlyLand():
"""
  控制无人机降落，降落后无人机桨叶停止，后续不能有飞行动作
"""

def FlyTimeGroup(name, minute,second):
"""
  无人机在此时开始执行所包含的动作，计时从起飞开始
  :param name:   时间组名称，字符串，不可以重复
  :param minute: 第N分钟，正整数
  :param second: 第N秒钟，0-60以内的正整数，
"""

def FlyAddMarkPoint(name, x, y, z):
"""
  添加标定点，此函数只是预设一个位置，无人机不执行飞行动作
  :param name: 标定点名称，字符串，标定点名称不可以重复
  :param x:    X轴坐标位置(厘米)，正整数
  :param y:    Y轴坐标位置(厘米)，正整数
  :param z:    Z轴坐标位置(厘米)，正整数
"""

def FlyToMarkPoint(name, second):
"""
  飞行至标定点，配合添加标定点使用，控制无人机飞行到预设的位置
  :param name:        标定点名称，字符串，必须已经添加标定点
  :param second:      飞行到此标定点用时(秒)，正整数
"""

def FlyMove(direction, distance, second):
"""
  向设置方向飞行
  :param direction:   飞行方向，取值范围1-6正整数，[1向前/2向后/3向右/4向左/5向上/6向下]
  :param distance:    飞行距离(厘米)，正整数 
  :param second:      飞行用时(秒)，正整数
"""

def FlyHover(second):
"""
  悬停，在当前位置停留
  :param second: 悬停时间(秒)，正整数
"""

def FlyRevolve(angle, second):
"""
  旋转，无人机旋转设定角度
  :param angle:       旋转角度，取值范围-360°--360°，正直为顺时针旋转，负值为逆时针旋转
  :param second:      旋转用时(秒)，正整数
"""

def FlySetLedMode(mode):
"""
  设置LED灯闪烁模式
  :param mode: 模式，取值范围1-5，[1七彩灯/2呼吸灯/3点亮/4熄灭/5跑马灯]
"""

def FlySetLedColor(color):
"""
  设置LED灯颜色
  :param color: 颜色，字符串，十六进制颜色值，例：红色#FF0000
"""

def FlyTo(x, y, z, second):
"""
  飞行到设定位置
  :param x:           X轴坐标位置(厘米)，正整数
  :param y:           Y轴坐标位置(厘米)，正整数
  :param z:           Z轴坐标位置(厘米)，正整数
  :param second:      飞行用时(秒)，正整数
"""