'use strict';
goog.require('Blockly.JavaScript');
 
Blockly.JavaScript['QzrobotStart'] = function(block) {
    //开始
    var code = Start() + '\n';
    return code;
};

Blockly.JavaScript['Block_Inittime'] = function(block) {
    var text_time = block.getFieldValue("time");
    var statements_functionintit = Blockly.JavaScript.statementToCode(block, 'functionIntit');
    var code = inittime(text_time) + '\n' + statements_functionintit;
    return code;
};
    
Blockly.JavaScript['QzrobotUnLock'] = function(block) {
    //解锁
    var code = UnLock() + '\n';
    return code;
};

Blockly.JavaScript['QzrobotLock'] = function(block) {
    //上锁
    var code = Lock() + '\n';
    return code;
};
//降落
Blockly.JavaScript['QzrobotLand'] = function(block) {
    var code = Land() + '\n';
    return code; //返回的函数
};

Blockly.JavaScript['QzrobotTakeOff'] = function(block) {
    var alt = block.getFieldValue('alt');
    var code = Takeoff(alt) + '\n';
    return code;
};

Blockly.JavaScript['SpecifiedpeedTakeOff'] = function(block) {
    var text_speed = block.getFieldValue('speed');
    var text_height = block.getFieldValue('height');
    var code = SpecifiedpeedTakeOff(text_speed,text_height) + '\n';
    return code;
};

Blockly.JavaScript['UAV_Marking_Point'] = function(block) {
    var name = block.getFieldValue('name');
    var px = block.getFieldValue('X');
    var py = block.getFieldValue('Y');
    var pz = block.getFieldValue('Z');
    var code = markingpoint(name,px,py,pz) + '\n';
    return code;
};

Blockly.JavaScript['UAV_MoveToPoint'] = function(block) {
    var point = block.getFieldValue('point');
    var code = Move2Marker(point) + '\n';
    return code;
};

Blockly.JavaScript['QzrobotHover'] = function(block) {
    //悬停
    var code = Hover() + '\n';
    return code; //返回的函数
};

//角速度
Blockly.JavaScript['QzrobotAngularVelocity'] = function(block) {
    var velocity =  block.getFieldValue('velocity');
    var turnDirection = block.getFieldValue('turnDirection');
    var angle = block.getFieldValue('angle');
    var code = Yawlr(velocity,turnDirection,angle) + '\n';;
    return code;
};

//指定速度//垂直指定方向//飞行指定距离
Blockly.JavaScript['QzrobotVerticalSpeed'] = function(block) {
    var hspeed = block.getFieldValue('VH');
     var dropdown_direction = block.getFieldValue('direction');
     var jl = block.getFieldValue('JL');
    var code = Moveud(hspeed,dropdown_direction,jl) + '\n';
    return code;
};


Blockly.JavaScript['QzrobotHorizontalSpeed'] = function(block) {
    var VHS = block.getFieldValue('VHS');
     var dropdown_direction = block.getFieldValue('direction');
     var JLX = block.getFieldValue('JLX');
    var code = Dctmove(VHS,dropdown_direction,JLX) + '\n';
    return code;
};


Blockly.JavaScript['QzrobotDirection'] = function(block) {
    var direction = block.getFieldValue('turnDirection');
    var code = Desnw(direction) + '\n';
    return code;
};

//水平速度 水平加速度
Blockly.JavaScript['QzrobotHorizontalSpeedcp'] = function(block) {
    var hspeed = block.getFieldValue('VH');
     var ha = block.getFieldValue('AH');
    var code = VelAccXY(hspeed,ha) + '\n';
    return code;
};

Blockly.JavaScript['QzrobotVerticalSpeedcp'] = function(block) {
    var hspeed = block.getFieldValue('VV');
    var ha = block.getFieldValue('AV');
    var code = MaxVelZ(hspeed,ha) + '\n';
    return code;
};

//指定速度飞行指定距离
Blockly.JavaScript['QzrobotSpecifiedSpeed'] = function(block) {
    var Gspeed = block.getFieldValue('speed');
    var jl = block.getFieldValue('JL');
    var code = Mspned(Gspeed,jl) + '\n';
    return code;
};


Blockly.JavaScript['QzrobotMove'] = function(block) {
    var px = block.getFieldValue('X');
    var py = block.getFieldValue('Y');
    var pz = block.getFieldValue('Z');
    var code = MoveDelta(px,py,pz) + '\n';
    return code;
};

Blockly.JavaScript['QzrobotPoint'] = function(block) {
    var px = block.getFieldValue('X');
    var py = block.getFieldValue('Y');
    var pz = block.getFieldValue('Z');
    var code = AddMark(px,py,pz) + '\n';
    return code;
};

/**
 * 显示板块
 * @param block
 * @returns {string}
 * @constructor
 */
Blockly.JavaScript['QzrobotAtomicLEDOn'] = function(block) {
    var text_id = block.getFieldValue('led');
    var text_color = block.getFieldValue('color');
    var code = SpecifySingle(text_id,text_color) + '\n';
    return code;
};

Blockly.JavaScript['QzrobotLed_Colour'] = function(block) {
    var dropdown_led = block.getFieldValue('led');
    var color1 = block.getFieldValue('color1');
    var text_led_color_time = block.getFieldValue('led_color_time');
    var code = LedColour(dropdown_led,color1,text_led_color_time)+'\n';
    return code;
};

Blockly.JavaScript['QzrobotLed_Colour_Right'] = function(block) {
    var dropdown_led = block.getFieldValue('led');
    var color = block.getFieldValue('color');
    var dropdown_bright = block.getFieldValue('bright');
    var text_led_color_time = block.getFieldValue('led_color_right_time');
    var code = LedColorRight(dropdown_led,color,dropdown_bright,text_led_color_time) + '\n';
    return code;
};

Blockly.JavaScript['QzrobotLEDSingleBlink'] = function(block) {
    var text_color1 = block.getFieldValue('color');
    var time = block.getFieldValue('time');
    var code = BlinkSingle(text_color1,time)+'\n';
    return code;
};

/**
 * 检测板块
 * @param block
 * @returns {string}
 * @constructor
 */
Blockly.JavaScript['UAV_DetectedData'] = function(block) {
    var dropdown_direction = block.getFieldValue('direction');
    var text_detecteddata = block.getFieldValue('detecteddata');
    var dropdown_envelements = block.getFieldValue('envelements');
    var code = detectedOne(dropdown_direction,text_detecteddata,dropdown_envelements) + '\n';
    return code;
};

Blockly.JavaScript['UAV_Dexecute_Drone'] = function(block) {
    var dropdown_direction = block.getFieldValue('direction');
    var dropdown_envelements = block.getFieldValue('envelements');
    var text_detecteddata = block.getFieldValue('detecteddata');
    var statements_execute = Blockly.Python.statementToCode(block, 'execute');
    var code = dexecutedrone(dropdown_direction,dropdown_envelements,text_detecteddata) + '\n';
    return code;
};

Blockly.JavaScript['UAV_Marking_Point'] = function(block) {
    var name = block.getFieldValue('name');
    var px = block.getFieldValue('X');
    var py = block.getFieldValue('Y');
    var pz = block.getFieldValue('Z');
    var code = markingpoint(name,px,py,pz) + '\n';
    return code;
};

Blockly.JavaScript['UAV_SetUnityPoint'] = function(block) {
    var name = block.getFieldValue('name');
    var flight = block.getFieldValue('flight');
    var code = SetUnityPoint(flight,name) + '\n';
    return code;
};

Blockly.JavaScript['UAV_FlightPOSXYZ'] = function(block) {
    var flight = block.getFieldValue('flight');
    var code = readxyz(flight) + '\n';
    return code;
};

Blockly.JavaScript['UAV_Drone_Flying_Style'] = function(block) {
    var flight = block.getFieldValue('flight');
    var code = droneFlyingStyle(flight) + '\n';
    return code;
};

Blockly.JavaScript['UAV_Drone_Style'] = function(block) {
    var flight = block.getFieldValue('flight');
    var direction = block.getFieldValue('direction');
    var ah = block.getFieldValue('AH');
    var code = flyingStyle(flight,direction,ah) + '\n';
    return code;
};

/**
 * 计时器板块
 * @param block
 * @returns {string}
 * @constructor
 */
Blockly.JavaScript['QzrobotTimerCounter_Start'] = function(block) {
    var code = TimerStart() + '\n';
    return code;
};

Blockly.JavaScript['QzrobotTimerCounter_Get'] = function(block) {

    var code = GetTimerPassMs() + '\n';
    return code;
};

Blockly.JavaScript['QzrobotTimerCounter_Clear'] = function(block) {
    var code = TimerClear() + '\n';
    return code;
};

Blockly.JavaScript['Block_Delay'] = function(block) {
    var dropdown_delay = block.getFieldValue('delay');
    var dropdown_time = block.getFieldValue('time');
    if(dropdown_delay==0)
        var code = Delay(dropdown_time) + '\n';
    else if(dropdown_delay==1)
        var code = Delay(dropdown_time*1000) + '\n';
    else if(dropdown_delay==2)
        var code = Delay(dropdown_time*60000) + '\n';

    return code;
};