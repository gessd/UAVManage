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

Blockly.Blocks['FlyTakeoff'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("起飞至")
            .appendField(new Blockly.FieldNumber(100, 0, getMaxZ(), 1), "height")
            .appendField("cm");
        this.setNextStatement(true, ["time"]);
        this.setColour('#FF6680');
        this.setTooltip('取值范围:0~'+getMaxZ());
    }
};

Blockly.Python['FlyTakeoff'] = function(block) {
    initHead("FlyTakeoff");
    var alt = block.getFieldValue("height");
    var code = 'FlyTakeoff('+ alt + ')' + '\n';
    return code;
};

Blockly.Blocks['FlyTimeGroup'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("开始时间 第")
            .appendField(new Blockly.FieldNumber(0, 0, 100, 1), "minute")
            .appendField("分")
            .appendField(new Blockly.FieldNumber(1, 0, 59, 1), "second")
            .appendField("秒")
        this.appendStatementInput("interiorfunction").setCheck(null);
        this.setPreviousStatement(true, "time");
        this.setNextStatement(true, "time");
        this.setColour('#FF6680');
    }
};

Blockly.Python['FlyTimeGroup'] = function(block) {
    initHead("FlyTimeGroup");
    var m = block.getFieldValue("minute");
    var s = block.getFieldValue("second");
    var ff = Blockly.Python.statementToCode(block, 'interiorfunction');
    var code = "FlyTimeGroup("+m+","+s+")\n";
    if(false == isEmpty(ff)) {
        code = code + "if True:\n"+ff;
    }
    return code;
};

Blockly.Blocks['FlyLand'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("降落 ");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#FF6680');
        this.setTooltip('原地降落');
    }
};

Blockly.Python['FlyLand'] = function(block) {
    initHead("FlyLand");
    var code = 'FlyLand()' + '\n';
    return code;
};

Blockly.Blocks['FlyAddMarkPoint'] = {
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
            .appendField("cm")
            .appendField("Y")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxY(), 1), "coordinateY")
            .appendField("cm")
            .appendField("Z")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxZ(), 1), "coordinateZ")
            .appendField("cm");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#008080');
        this.setTooltip('标定点名称不可以重复,最长10个字符');
    }
};

Blockly.Python['FlyAddMarkPoint'] = function(block) {
    initHead("FlyAddMarkPoint");
    var name = block.getFieldValue('name');
    var px = block.getFieldValue("coordinateX");
    var py = block.getFieldValue("coordinateY");
    var pz = block.getFieldValue("coordinateZ");
    var code = 'FlyAddMarkPoint(\'' + name + '\',' + px + ',' + py + ',' + pz + ')' + '\n';
    return code;
};

Blockly.Blocks['FlyToMarkPoint'] = {
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
            .appendField(field, "mark");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#3B8CFF');
    }
};
Blockly.Python['FlyToMarkPoint'] = function(block) {
    initHead("FlyToMarkPoint");
    var point = block.getFieldValue('mark');
    var code = 'FlyToMarkPoint(\'' + point + '\')' + '\n';
    return code;
};

Blockly.Blocks['FlyTo'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行到")
            .appendField(" X")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxX(), 1), "coordinateX")
            .appendField("cm")
            .appendField("Y")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxY(), 1), "coordinateY")
            .appendField("cm")
            .appendField("Z")
            .appendField(new Blockly.FieldNumber(100, 100, getMaxZ(), 1), "coordinateZ")
            .appendField("cm");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("[100<X<"+getMaxX()+"] [100<Y<"+getMaxY()+"] [100<Z<"+getMaxZ()+"]");
        this.setHelpUrl("");
    }
};

Blockly.Python['FlyTo'] = function(block) {
    initHead("FlyTo");
    var px = block.getFieldValue("coordinateX");
    var py = block.getFieldValue("coordinateY");
    var pz = block.getFieldValue("coordinateZ");
    var code = 'FlyTo(' + px + ',' + py + ',' + pz +')' + '\n';
    return code;
};

Blockly.Blocks['FlySetSpeed'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("速度")
            .appendField(new Blockly.FieldNumber(60, 1, 500, 1), "speed")
            .appendField("cm/s")
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setInputsInline(true);
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['FlySetSpeed'] = function(block) {
    initHead("FlySetSpeed");
    var s = block.getFieldValue("speed");
    var code = 'FlySetSpeed(' + s +')' + '\n';
    return code;
};

Blockly.Python['FlySetSpeed'] = function(block) {
    initHead("FlySetSpeed");
    var s = block.getFieldValue("speed");
    var code = 'FlySetSpeed(' + s +')' + '\n';
    return code;
};
Blockly.Blocks['FlyHover'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("悬停")
            .appendField(new Blockly.FieldNumber(1, 1, 1000, 1), "hover")
        this.appendDummyInput()
			.appendField(new Blockly.FieldDropdown([
				["秒", "1"],
                ["毫秒", "2"]
             ]), "unit");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};
Blockly.Python['FlyHover'] = function(block) {
    initHead("FlyHover");
    var t = block.getFieldValue('hover');
    var u = block.getFieldValue('unit');
    if(1 == u){
        t = t*1000;
    }
    var code = 'FlyHover('+t+')' + '\n';
    return code; 
};

Blockly.Blocks['FlyRevolve'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("向")
			.appendField(new Blockly.FieldDropdown([
				["右", "right"],
				["左", "left"],
             ]), "direction");
        this.appendDummyInput()
             .appendField('角度:')
             .appendField(new Blockly.FieldAngle(90), 'revolve');
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['FlyRevolve'] = function(block) {
    initHead("FlyRevolve");
    var r = block.getFieldValue('revolve');
    var d = block.getFieldValue('direction');
    if("right" == d){
        return 'FlyRevolve('+r+')' + '\n';
    } else if("left" == d){
        return 'FlyRevolve('+-r+')' + '\n';
    }
};

Blockly.Blocks['FlyMove'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("向")
			.appendField(new Blockly.FieldDropdown([
				["前", "1"],
				["后", "2"],
                ["右", "3"],
                ["左", "4"],
                ["上", "5"],
				["下", "6"],
             ]), "direction")
             .appendField("飞")
             .appendField(new Blockly.FieldNumber(100, 1, getMaxX(), 1), "distance")
             .appendField("cm")
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['FlyMove'] = function(block) {
    initHead("FlyMove");
    var d = block.getFieldValue('direction');
    var s = block.getFieldValue('distance');
    return 'FlyMove('+d+','+s+')\n';
};

Blockly.Blocks['FlySetLed'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("设置LED灯为")
			.appendField(new Blockly.FieldDropdown([
				["模式1", "1"],
				["模式2", "2"],
                ["模式3", "3"],
                ["模式4", "4"],
                ["模式5", "5"],
                ["模式6", "6"],
                ["模式7", "7"],
             ]), "mode");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('');
        this.setInputsInline(true);
    }
};

Blockly.Python['FlySetLed'] = function(block) {
    initHead("FlySetLed");
    var m = block.getFieldValue('mode');
    return 'FlySetLed('+m+')\n';
};