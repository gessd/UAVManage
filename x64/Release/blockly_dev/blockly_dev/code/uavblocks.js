'use strict';

goog.require('Blockly.Blocks');
goog.require('Blockly');

function isEmpty(v) {
    switch (typeof v) {
    case 'undefined':
        return true;
    case 'string':
        if (v.replace(/(^[ \t\n\r]*)|([ \t\n\r]*$)/g, '').length == 0) return true;
        break;
    case 'boolean':
        if (!v) return true;
        break;
    case 'number':
        if (0 === v || isNaN(v)) return true;
        break;
    case 'object':
        if (null === v || v.length === 0) return true;
        for (var i in v) {
            return false;
        }
        return true;
    }
    return false;
}

function getMaxX(){
    return window.spaceX;
}
function getMaxY(){
    return window.spaceY;
}
function getMaxZ(){
    return window.spaceZ;
}

function initHead(name){
    var head = Blockly.Python.definitions_.import_PythonWrap;
    if(isEmpty(head)){
        Blockly.Python.definitions_.import_PythonWrap = "#coding=utf-8\nfrom QZAPI import "  +name + "\n";
    } else {
        if(head.indexOf(name)>=0) return;
        Blockly.Python.definitions_.import_PythonWrap += "from QZAPI import " +name+ "\n";
    }
}
function addHead(){
    Blockly.Python.definitions_.import_PythonWrap = "#coding=utf-8\nfrom QZAPI import *";
}

Blockly.Blocks['Fly_Takeoff'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("起飞至")
            .appendField(new Blockly.FieldNumber(100, 100, 100, 1), "height")
            .appendField("厘米")
            .appendField("用时")
            .appendField(new Blockly.FieldNumber(5, 2, 10, 1), "time")
            .appendField("秒");
        this.setNextStatement(true, ["time"]);
        //this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#FF6680');
        this.setTooltip('起飞至高度固定为100厘米，用时范围2~10秒');
    }
};

Blockly.Python['Fly_Takeoff'] = function(block) {
    addHead();
    var alt = block.getFieldValue("height");
    var t = block.getFieldValue('time');
    var code = 'Fly_Takeoff('+ alt + ',' + t + ',\'' +block.id + '\''+ ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_TakeoffDelay'] = {
    init: function() {
        this.appendDummyInput("flystart")
            .appendField("地面停留")
            .appendField(new Blockly.FieldNumber(0, 0, 300, 1), "delay")
            .appendField("秒后")
            .appendField("起飞至")
            .appendField(new Blockly.FieldNumber(100, 100, 100, 1), "height")
            .appendField("厘米")
            .appendField("用时")
            .appendField(new Blockly.FieldNumber(5, 2, 10, 1), "time")
            .appendField("秒");
        this.setNextStatement(true, ["time"]);
        //this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#FF6680');
        this.setTooltip('起飞至高度固定为100厘米，用时范围2~10秒');
    }
};

Blockly.Python['Fly_TakeoffDelay'] = function(block) {
    addHead();
    var delay = block.getFieldValue("delay");
    var alt = block.getFieldValue("height");
    var t = block.getFieldValue('time');
    var code = 'Fly_TakeoffDelay('+ delay + ',' +alt + ',' + t + ',\'' +block.id + '\''+ ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_TimeGroup'] = {
    init: function() {
        var validator = function(newValue) {
            //限制字符串长度
            if(newValue.length>10) return newValue.substring(0,10);
            return newValue;
        };
        var field = new Blockly.FieldTextInput("1");
        field.setValidator(validator);
        this.appendDummyInput()
            .appendField("动作")
            .appendField(field, "GroupName")
            .appendField("开始时间 第")
            .appendField(new Blockly.FieldNumber(0, 0, 100, 1), "minute")
            .appendField("分")
            .appendField(new Blockly.FieldNumber(5, 0, 59, 1), "second")
            .appendField("秒", "test")
        this.appendStatementInput("interiorfunction").setCheck(null);
        this.setPreviousStatement(true, "time");
        this.setNextStatement(true, "time");
        this.setColour('#FF6680');
        this.setTooltip('动作名称最长10个汉字，不可以重复，时间取值范围:0~100分 0~59秒');
    }
};

Blockly.Python['Fly_TimeGroup'] = function(block) {
    addHead();
    var name = block.getFieldValue("GroupName");
    var m = block.getFieldValue("minute");
    var s = block.getFieldValue("second");
    var ff = Blockly.Python.statementToCode(block, 'interiorfunction');
    var code = 'Fly_TimeGroup(\'' + name + '\',' + m + ',' + s + ',\'' +block.id + '\''+')\n';
    if(false == isEmpty(ff)) {
        code = code + "if True:\n"+ff;
    }
    return code;
};

Blockly.Blocks['Fly_Land'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("降落 ");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#FF6680');
        this.setTooltip('原地降落');
    }
};

Blockly.Python['Fly_Land'] = function(block) {
    addHead();
    var code = 'Fly_Land('+ '\'' +block.id + '\''+')' + '\n';
    return code;
};

Blockly.Blocks['Fly_AddMarkPoint'] = {
    init: function() {
        var validator = function(newValue) {
            //限制字符串长度
            if(newValue.length>10) return newValue.substring(0,10);
            return newValue;
        };
        var field = new Blockly.FieldTextInput("a");
        field.setValidator(validator);
        this.appendDummyInput()
            .appendField("添加标定点 ")
            .appendField("名称")
            .appendField(field, "name")
            .appendField(" X")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxX(), 1), "coordinateX")
            .appendField("厘米")
            .appendField("Y")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxY(), 1), "coordinateY")
            .appendField("厘米")
            .appendField("Z")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxZ(), 1), "coordinateZ")
            .appendField("厘米");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#3B8CFF');
        this.setTooltip("名称不可以重复，最长10个汉字，位置范围"+"[100<X<"+getMaxX()+"] [100<Y<"+getMaxY()+"] [100<Z<"+getMaxZ()+"]");
    }
};

Blockly.Python['Fly_AddMarkPoint'] = function(block) {
    addHead();
    var name = block.getFieldValue('name');
    var px = block.getFieldValue("coordinateX");
    var py = block.getFieldValue("coordinateY");
    var pz = block.getFieldValue("coordinateZ");
    var code = 'Fly_AddMarkPoint(\'' + name + '\',' + px + ',' + py + ',' + pz + ',\'' + block.id + '\'' + ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_ToMarkPoint'] = {
    init: function() {
        var validator = function(newValue) {
            //限制字符串长度
            if(newValue.length>10) return newValue.substring(0,10);
            return newValue;
        };
        var field = new Blockly.FieldTextInput("a");
        field.setValidator(validator);
        this.appendDummyInput()
            .appendField("飞至标定点")
            .appendField(field, "mark")
            .appendField("用时")
            .appendField(new Blockly.FieldNumber(1, 1, 60, 1), "time")
            .appendField(new Blockly.FieldDropdown([
                 ["秒", "1"]
              ]), "unit");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#3B8CFF');
        this.setTooltip("使用前需要提前添加标定点，用时范围1~60秒");
    }
};
Blockly.Python['Fly_ToMarkPoint'] = function(block) {
    addHead();
    var point = block.getFieldValue('mark');
    var t = block.getFieldValue('time');
    var u = block.getFieldValue('unit');
    var code = 'Fly_ToMarkPoint(\'' + point + '\',' + t + ',\'' +block.id + '\'' + ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_To'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行到")
            .appendField("X")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxX(), 1), "coordinateX")
            .appendField("厘米")
            .appendField("Y")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxY(), 1), "coordinateY")
            .appendField("厘米")
            .appendField("Z")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxZ(), 1), "coordinateZ")
            .appendField("厘米")
            .appendField("用时")
            .appendField(new Blockly.FieldNumber(1, 1, 60, 1), "time")
            .appendField(new Blockly.FieldDropdown([
                 ["秒", "1"]
              ]), "unit");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("位置范围[100<X<"+getMaxX()+"] [100<Y<"+getMaxY()+"] [100<Z<"+getMaxZ()+"]，用时范围1~60秒");
        this.setHelpUrl("");
    }
};

Blockly.Python['Fly_To'] = function(block) {
    addHead();
    var px = block.getFieldValue("coordinateX");
    var py = block.getFieldValue("coordinateY");
    var pz = block.getFieldValue("coordinateZ");
    var t = block.getFieldValue("time");
    var u = block.getFieldValue('unit');
    var code = 'Fly_To(' + px + ',' + py + ',' + pz +',' + t + ',\'' + block.id + '\'' + ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_ToNumber'] = {
    init: function() {
        this.appendDummyInput()
            .appendField('飞行到');
        this.appendValueInput('X')
            .setCheck('Number')
            .appendField('X');
        this.appendValueInput('Y')
            .setCheck('Number')
            .appendField('Y');
        this.appendValueInput('Z')
            .setCheck('Number')
            .appendField('Z');
        this.appendValueInput('time')
            .setCheck('Number')
            .appendField('用时');
        this.appendDummyInput()
             .appendField(new Blockly.FieldDropdown([
                 ["秒", "1"]
              ]), "unit");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('自定义飞行位置');
        this.setHelpUrl('');
    }
};
Blockly.Python['Fly_ToNumber'] = function(block) {
    addHead();
    var px = Blockly.Python.valueToCode(block, 'X', Blockly.Python.ORDER_NONE);
	var py = Blockly.Python.valueToCode(block, 'Y', Blockly.Python.ORDER_NONE);
	var pz = Blockly.Python.valueToCode(block, 'Z', Blockly.Python.ORDER_NONE);
    var t = Blockly.Python.valueToCode(block, 'time', Blockly.Python.ORDER_NONE);
    var u = block.getFieldValue('unit');
    var code = 'Fly_To(' + px + ',' + py + ',' + pz + ',' + t + ',\'' + block.id + '\'' + ')' + '\n';
    return code;
};

Blockly.Blocks['Fly_SetSpeed'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行速度每秒")
            .appendField(new Blockly.FieldNumber(60, 10, 200, 1), "speed")
            .appendField("厘米")
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setInputsInline(true);
        this.setTooltip('取值范围:10~200');
        this.setHelpUrl("");
    }
};

Blockly.Python['Fly_SetSpeed'] = function(block) {
    addHead();
    var s = block.getFieldValue("speed");
    var code = 'Fly_SetSpeed(' + s +')' + '\n';
    return code;
};

Blockly.Blocks['Fly_Hover'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("悬停")
            .appendField(new Blockly.FieldNumber(1, 1, 60, 1), "hover")
        this.appendDummyInput()
			.appendField(new Blockly.FieldDropdown([
				["秒", "1"]
             ]), "unit");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('悬停时间范围:1~60秒');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};
Blockly.Python['Fly_Hover'] = function(block) {
    addHead();
    var t = block.getFieldValue('hover');
    var u = block.getFieldValue('unit');
    var code = 'Fly_Hover(' +t + ',\'' + block.id + '\'' + ')' + '\n';
    return code; 
};

Blockly.Blocks['Fly_Revolve'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("向")
			.appendField(new Blockly.FieldDropdown([
				["右", "right"],
				["左", "left"],
             ]), "direction");
        this.appendDummyInput()
             .appendField('旋转 角度:')
             .appendField(new Blockly.FieldAngle(90), 'revolve')
             .appendField(" 用时")
             .appendField(new Blockly.FieldNumber(1, 1, 60, 1), "time");
        this.appendDummyInput()
             .appendField(new Blockly.FieldDropdown([
                 ["秒", "1"]
              ]), "unit");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('旋转角度0~360度，用时范围1~60秒');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['Fly_Revolve'] = function(block) {
    addHead();
    var r = block.getFieldValue('revolve');
    var d = block.getFieldValue('direction');
    var t = block.getFieldValue('time');
    var u = block.getFieldValue('unit');
    if("right" == d){
        return 'Fly_Revolve('+r+','+t+ ',\'' + block.id + '\'' + ')' + '\n';
    } else if("left" == d){
        return 'Fly_Revolve('+-r+','+t+ ',\'' + block.id + '\'' + ')' + '\n';
    }
};

Blockly.Blocks['Fly_Move'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("向")
			.appendField(new Blockly.FieldDropdown([
				["前", "1"],
				["后", "2"],
                ["左", "4"],
                ["右", "3"],
                ["上", "5"],
				["下", "6"],
             ]), "direction")
             .appendField("飞")
             .appendField(new Blockly.FieldNumber(100, 10, getMaxZ(), 1), "distance")
             .appendField("厘米")
             .appendField(" 用时")
             .appendField(new Blockly.FieldNumber(1, 1, 60, 1), "time");
        this.appendDummyInput()
             .appendField(new Blockly.FieldDropdown([
                 ["秒", "1"]
              ]), "unit");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("位置范围10~"+getMaxZ()+"厘米，用时范围1~60秒");
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['Fly_Move'] = function(block) {
    addHead();
    var d = block.getFieldValue('direction');
    var s = block.getFieldValue('distance');
    var t = block.getFieldValue('time');
    var u = block.getFieldValue('unit');
    return 'Fly_Move('+d+','+s+','+t+ ',\'' + block.id + '\'' + ')\n';
};

Blockly.Blocks['Fly_SetLedMode'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("设置LED灯闪烁模式为")
			.appendField(new Blockly.FieldDropdown([
				["七彩灯", "6"],
				["呼吸灯", "7"],
                ["点亮",   "8"],
                ["熄灭",   "9"],
                ["跑马灯", "10"],
             ]), "mode");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['Fly_SetLedMode'] = function(block) {
    addHead();
    var m = block.getFieldValue('mode');
    return 'Fly_SetLedMode(' + m + ',\'' + block.id + '\'' + ')\n';
};

Blockly.Blocks['Fly_SetLedColor'] = {
    init: function() {
        this.appendDummyInput()
        .appendField("设置LED灯颜色")
        .appendField(new Blockly.FieldColour("#0000FF"), "color");	
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('选择预定义的颜色');
        this.setHelpUrl('');
        this.setInputsInline(true);
      }
};

Blockly.Python['Fly_SetLedColor'] = function(block) {
    addHead();
    var color = block.getFieldValue('color');
    return 'Fly_SetLedColor('+ '\'' + color + '\'' + ',\'' + block.id + '\'' + ')\n';
};
