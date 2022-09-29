'use strict';
goog.require('Blockly.JavaScript');

var mavlinkcmd = [];
    //开始
function Start(){
    var start_data = {
        "command":300
    };
    mavlinkcmd.push(start_data);
    console.log(JSON.stringify(mavlinkcmd));
}

//start at
function inittime(text_time){
    var init_data = {
        "command":1,
        "text_time":text_time
    };
    mavlinkcmd.push(init_data);
    console.log(JSON.stringify(mavlinkcmd));
}

//解锁
function UnLock(){
    var mav_UnLock =
    {
        "command": 400,
        "Lock":0
    };
    mavlinkcmd.push(mav_UnLock);
    console.log(JSON.stringify(mavlinkcmd));
}

//上锁
function Lock(){
    var mav_Lock  =
       {
         "command": 400,
         "Lock":1
       };
    mavlinkcmd.push(mav_Lock);
    console.log(JSON.stringify(mavlinkcmd));
}

//降落
function Land(){
    var mav_Land  =
    {
     "command": 21
    };
   mavlinkcmd.push(mav_Land);
   console.log(JSON.stringify(mavlinkcmd));
}

function TimerStart(){
    var timerStart_data = {
        "command":100
    };
    mavlinkcmd.push(timerStart_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function GetTimerPassMs(){
    var gettimer_data = {
        "command":100
    };
    mavlinkcmd.push(gettimer_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function TimerClear(){
    var timer_data = {
        "command":100
    };
    mavlinkcmd.push(timer_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function Delay(dropdown_time){
    var dropdowntime_data = {
        "command":100,
        "dropdown_time":dropdown_time
    };
    mavlinkcmd.push(dropdowntime_data);
    console.log(JSON.stringify(mavlinkcmd));
}

//起飞至
function Takeoff(alt){
    var mav_Takeoff = {
    "command": 22,
    "height": alt
    };
    mavlinkcmd.push(mav_Takeoff);
    console.log(JSON.stringify(mavlinkcmd));
}

function SpecifiedpeedTakeOff(text_speed,text_height){
    var specifiedpeed_data = {
        "command":100,
        "text_speed":text_speed,
        "text_height":text_height
    };
    mavlinkcmd.push(specifiedpeed_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function Hover(){
    //悬停
    var mav_Hover = {
    "command": 17
    };
    mavlinkcmd.push(mav_Hover);
    console.log(JSON.stringify(mavlinkcmd));
}

//向上向下
function Moveud(hspeed,direction,jl){
  //垂直指定方向飞行指定距离
  var mav_Moveud = {
    "command": 16,
    "speed"  : hspeed,
    "direction" : direction,
    "distance" :  jl
   };
   mavlinkcmd.push(mav_Moveud);
   console.log(JSON.stringify(mavlinkcmd));
}

//向左转向右转
function Yawlr(velocity,turnDirection,angle){
  //向左或者向右转多少度
    var mav_Yawlr = {
      "command": 18,
      "speed" : velocity,
      "direction" :  turnDirection,
      "angle"     :   angle
   };
    mavlinkcmd.push(mav_Yawlr);
    console.log(JSON.stringify(mavlinkcmd));
}

//指定速度//水平指定方向前后//飞行指定距离//
function Dctmove(vhs,dropdown_direction,JLX){
     var dctmove_data = {
     // "identification": "action",
     "command": 16,
     "speed" :  vhs,
     "direction" : dropdown_direction,
     "distance" : JLX
      };
     mavlinkcmd.push(dctmove_data);
     console.log(JSON.stringify(mavlinkcmd));
}

function Desnw(direction) {
       var dir_esn ={
        "command":115,
        "direction" : direction
       };
       mavlinkcmd.push(dir_esn);
       console.log(JSON.stringify(mavlinkcmd));
}

//水平速度 水平加速度
function VelAccXY(hspeed,ha){
      var dir_esn ={
        "command":178,
        "speed" : hspeed,
        "acceleration" : ha
       };
       mavlinkcmd.push(dir_esn);
       console.log(JSON.stringify(mavlinkcmd));
}

function MaxVelZ(hspeed,ha){
    var dir_esn ={
        "command":178,
        "speed" : hspeed,
        "acceleration" : ha
    };
    mavlinkcmd.push(dir_esn);
    console.log(JSON.stringify(mavlinkcmd));
}

//指定速度到指定距离(速度/飞行距离)
function Mspned(Gspeed,jl){
     var dir_mspnde ={
    "command":178,
    "speed" : Gspeed,
    "distance" : jl
     };
     mavlinkcmd.push(dir_mspnde);
     console.log(JSON.stringify(mavlinkcmd));
}

//xyz坐标移动
function MoveDelta(px,py,pz){
  var mov_pxyz={
    "command":16,
    "lon" :px,
    "lat" :py,
    "lzt" :pz
  };
  mavlinkcmd.push(mov_pxyz);
  console.log(JSON.stringify(mavlinkcmd));
}

function AddMark(px,py,pz){
    var mov_addmark={
    "command":16,
    "lon" :px,
    "lat" :py,
    "lzt" :pz
  };
   mavlinkcmd.push(mov_addmark);
   console.log(JSON.stringify(mavlinkcmd));
}

//显示
function SpecifySingle(text_id,text_color){
    var specify_data = {
        "command":100,
        "text_id":text_id,
        "text_color":text_color
    };
    mavlinkcmd.push(specify_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function LedColour(dropdown_led,color1,text_led_color_time){
    var ledcolour_data = {
        "command":101,
        "text_id":dropdown_led,
        "color1":color1,
        "text_led_color_time":text_led_color_time
    };
    mavlinkcmd.push(ledcolour_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function LedColorRight(dropdown_led,color,dropdown_bright,text_led_color_time){
    var ledcolorright_data = {
        "command":102,
        "text_id":dropdown_led,
        "color":color,
        "dropdown_bright":dropdown_bright,
        "text_led_color_time":text_led_color_time
    };
    mavlinkcmd.push(ledcolorright_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function BlinkSingle(text_color1,time){
    var blinksingle_data = {
        "command":100,
        "time":time,
        "color":text_color1
    };
    mavlinkcmd.push(blinksingle_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function detectedOne(dropdown_direction,text_detecteddata,dropdown_envelements){
    var detectedone_data = {
        "command":103,
        "dropdown_direction":dropdown_direction,
        "text_detecteddata":text_detecteddata,
        "dropdown_envelements":dropdown_envelements
    };
    mavlinkcmd.push(detectedone_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function dexecutedrone(dropdown_direction,dropdown_envelements,text_detecteddata){
    var dexecutedrone_data = {
        "command":104,
        "dropdown_direction":dropdown_direction,
        "dropdown_envelements":dropdown_envelements,
        "text_detecteddata":text_detecteddata
    };
    mavlinkcmd.push(dexecutedrone_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function markingpoint(name,px,py,pz){
    var markingpoint_data = {
        "command":105,
        "name":name,
        "px":px,
        "py":py,
        "pz":pz
    };
    mavlinkcmd.push(markingpoint_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function SetUnityPoint(flight,name){
    var point_data = {
        "command":105,
        "name":name,
        "flight":flight
    };
    mavlinkcmd.push(point_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function readxyz(flight){
    var readxyz_data = {
        "command":105,
        "flight":flight
    };
    mavlinkcmd.push(readxyz_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function droneFlyingStyle(flight){
    var droneflying_data = {
        "command":105,
        "flight":flight
    };
    mavlinkcmd.push(droneflying_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function flyingStyle(flight,direction,ah){
    var droneflying_data = {
        "command":105,
        "flight":flight,
        "direction":direction,
        "ah":ah
    };
    mavlinkcmd.push(droneflying_data);
    console.log(JSON.stringify(mavlinkcmd));
}

function Move2Marker(point){
    var move2marker_data = {
        "command":106,
        "point":point
    };
    mavlinkcmd.push(move2marker_data);
    console.log(JSON.stringify(mavlinkcmd));
}

var url = "ws://127.0.0.1:25252";
//兼容 FireFox
if ("WebSocket" in window) {
   var socket = new WebSocket(url);
} else if ("MozWebSocket" in window) {
   var socket = new MozWebSocket(url);
}

socket.onopen = function(event) {

    // console.log("connected");
    // socket.send("client say wuyanming hello\n");
    // alert('连接');
};

socket.onmessage = function(event) {
    //当客户端收到服务端发来的消息时，会触发onmessage事件，参数event.data中包含server传输过来的数据
    var content = event.data;//获取消息
    //if(-1 != content.search("<xml")){
    //    Code.workspace.clear();
    //    var xml = Blockly.Xml.textToDom(content);
    //    Blockly.Xml.domToWorkspace(xml, Code.workspace);
    //} 
    var xmldata = "xmlData";
    var xmlcode = document.getElementById("xmldata").value;
    var pythondata = "pythonData";
    var pythoncode = document.getElementById("content_python_text").value;
    socket.send(xmldata +'\n'+xmlcode+'\n'+pythondata +'\n'+pythoncode);
    mavlinkcmd = [];
    if(content != "save"){
        Code.workspace.clear();
    }
    if(content != "null"){
        var xml = Blockly.Xml.textToDom(content);
        Blockly.Xml.domToWorkspace(xml, Code.workspace);
    }
};
socket.onclose = function(evt)
{
  //console.log("WebSocketClosed!");
};

socket.onerror = function(evt)
{
  //console.log("WebSocketError!");
};

function socektmavlink(){
    //var jsondata = "jsonData";
    //var mavdata = JSON.stringify(mavlinkcmd);
    //var xmldata = "xmlData";
    //var xmlcode = Blockly.Xml.domToPrettyText(Blockly.Xml.workspaceToDom(Code.workspace));
    //var pythondata = "pythonData";
    //var pythoncode = Blockly.Python.workspaceToCode(Code.workspace);
    //socket.send(jsondata +'\n'+mavdata +'\n'+xmldata +'\n'+xmlcode+'\n'+pythondata +'\n'+pythoncode);
    //socket.send(jsondata + '\n' + mavdata)
    //mavlinkcmd = [];
    //
    //Code.workspace.clear();
    //var content = document.getElementById("xmldata").value;
    //var xml = Blockly.Xml.textToDom(content);
    //Blockly.Xml.domToWorkspace(xml, Code.workspace);
}