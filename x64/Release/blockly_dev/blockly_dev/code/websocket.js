'use strict';
goog.require('Blockly.JavaScript');
var url = "ws://127.0.0.1:25252";
window.spaceX = 9000;
window.spaceY = 9000;
window.spaceZ = 500;

window.color = 0;
window.tipBlock = 0;
window.timerid = 0;
window.timerNumber = 0;

//兼容 FireFox
if ("WebSocket" in window) {
   var socket = new WebSocket(url);
} else if ("MozWebSocket" in window) {
   var socket = new MozWebSocket(url);
}

socket.onopen = function(event)
{
    console.log("socket connected");
};

socket.onmessage = function(event) {
    //当客户端收到服务端发来的消息时，会触发onmessage事件，参数event.data中包含server传输过来的数据
    var content = event.data;//获取消息
    console.log("收到服务端消息");
    try{
        //解析json有可能出错
        var jsonObject= JSON.parse(content);
        var msgID = jsonObject.msgID;
        console.log("解析json " + msgID);
        clearTipBlocklyColor();
        if(1 == msgID){
            console.log("更新编程区域");
            Code.workspace.clear();
            document.getElementById("tab_blocks").click();
            var xml = Blockly.Xml.textToDom(jsonObject.xml);
            Blockly.Xml.domToWorkspace(xml, Code.workspace);
            var name = jsonObject.name;
            console.log("设备名称:"+name);
            document.getElementById("deviceName").textContent = name;
            var pythonCode = jsonObject.pythonCode;
            if (typeof (pythonCode) == "undefined") {
                document.getElementById("tab_blocks").click();
                document.getElementById("tab_blocks").style.display = "";
                ace.edit("content_python").setValue("", 1);
            } else {
                document.getElementById("tab_python").click();
                ace.edit("content_python").setValue(pythonCode, 1);
            }
            //清空回撤功能数据
            Code.workspace.clearUndo();
        } else if(2 == msgID){
            console.log("清空编程区域");
            document.getElementById("tab_blocks").click();
			      document.getElementById("deviceName").textContent = "";
            Code.workspace.clear();
            //清空回撤功能数据
            Code.workspace.clearUndo();
        } else if(3 == msgID){
            console.log("更新空间范围");
            window.spaceX = jsonObject.x-100;
            window.spaceY = jsonObject.y-100;
            window.spaceZ = jsonObject.z;
            console.log("x:"+window.spaceX+" y:"+window.spaceY+" z:"+window.spaceZ);
        } else if(5 == msgID){
            console.log("定位积木块"+jsonObject.id);
            blockFlicker(jsonObject.id);
        }
    }catch(err){
    }
};
socket.onclose = function(evt)
{
  console.log("socket WebSocketClosed!");
};

socket.onerror = function(evt)
{
  console.log("socket WebSocketError!");
};

function blockFlicker(bid){
  var len = Code.workspace.getAllBlocks(false).length;
  console.log(len+"条积木块 查找的积木块"+bid);
  var blocks = Code.workspace.getAllBlocks(false);
  for (var i = 0; i < blocks.length; i++) {
    var bb = blocks[i];
    console.log("积木块ID" + bb.id);
    if(bid != bb.id) continue;
    console.log("找到对应ID积木块\n"+bb);
    window.tipBlock = bb;
    bb.select();
    window.color = bb.getColour();
    bb.setColour("#FF0000");
    window.timerid = setInterval(setTipBlocklyColor, 500);
    console.log(window.color + "原本颜色记录 开启定时闪烁 " + window.timerid);
    break;
  }
}

function setTipBlocklyColor(){
  console.log(window.timerid + " 定时器到时 " + window.timerNumber);
  window.timerNumber++;
  //需要先判断积木块是否还存在或改变
  var current = window.tipBlock.getColour();
  if(current == window.color){
    window.tipBlock.setColour("#FF0000");
  } else {
    window.tipBlock.setColour(window.color);
  }
  if(window.timerNumber>6){
    clearTipBlocklyColor();
  }
}

function clearTipBlocklyColor(){
  if(window.timerid != 0){
    console.log("清空积木块闪烁状态");
    clearInterval(window.timerid);
    window.tipBlock.setColour(window.color);
    window.color = 0;
    window.tipBlock = 0;
    window.timerid = 0;
    window.timerNumber = 0;
  }
}