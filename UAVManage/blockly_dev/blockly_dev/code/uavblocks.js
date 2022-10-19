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

function initHead(fun){
    var head = Blockly.Python.definitions_.import_PythonWrap;
    if(isEmpty(head)){
        Blockly.Python.definitions_.import_PythonWrap = "#coding=utf-8\nfrom QZAPI import "  +fun + "\n";
    } else {
        Blockly.Python.definitions_.import_PythonWrap += "from QZAPI import " +fun+ "\n";
    }
}

Blockly.Blocks['FlyTakeoff'] = {
    init: function() {
        this.appendValueInput("height")
            .setCheck("Number")
            .appendField("起飞至");
        this.appendDummyInput()
            .appendField("cm高度");
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction", "time"]);
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['FlyTakeoff'] = function(block) {
    //起飞至
    initHead("FlyTakeoff");
    var alt = Blockly.Python.valueToCode(this,'height',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyTakeoff('+ alt + ')' + '\n';
    return code;
};

Blockly.Blocks['FlyTimeGroup'] = {
    init: function() {
        this.appendDummyInput()
           .appendField("开始时间 第");
        this.appendValueInput('minute').setCheck('Number');
        this.appendDummyInput().appendField("分钟");
        this.appendValueInput('second').setCheck('Number');
        this.appendDummyInput().appendField("秒");
        this.appendStatementInput("interiorfunction").setCheck(null);
        this.setPreviousStatement(true, "time");
        this.setNextStatement(true, "time");
        this.setColour('#FF6680');
    }
};

Blockly.Python['FlyTimeGroup'] = function(block) {
    initHead("FlyTimeGroup");
    var m = Blockly.Python.valueToCode(block, 'minute', Blockly.Python.ORDER_NONE);
    var s = Blockly.Python.valueToCode(block, 'second', Blockly.Python.ORDER_NONE);
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
            .appendField("降落 ")
        this.setPreviousStatement(true);
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['FlyLand'] = function(block) {
    initHead("FlyLand");
    var code = 'FlyLand()' + '\n';
    return code; //返回的函数
};

Blockly.Blocks['FlyTo'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行到");
        this.appendValueInput("coordinateX")
            .setCheck("Number")
            .appendField("X");
        this.appendDummyInput()
            .appendField("cm");
        this.appendValueInput("coordinateY")
            .setCheck("Number")
            .appendField("Y");
        this.appendDummyInput()
            .appendField("cm");
        this.appendValueInput("coordinateZ")
            .setCheck("Number")
            .appendField("Z");
        this.appendDummyInput()
            .appendField("cm");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['FlyTo'] = function(block) {
    initHead("FlyTo");
    var px = Blockly.Python.valueToCode(this,'coordinateX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'coordinateY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'coordinateZ',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyTo(' + px + ',' + py + ',' + pz +')' + '\n';
    return code;
};

Blockly.Blocks['FlySetSpeed'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("");
        this.setHelpUrl("");
        this.setInputsInline(true);
    }
};

Blockly.Python['FlySetSpeed'] = function(block) {
    initHead("FlySetSpeed");
    var ps = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlySetSpeed(' + ps +')' + '\n';
    return code;
};

Blockly.Blocks['FlyHover'] = {
    init: function() {
        this.appendValueInput("hover")
            .setCheck("Number")
            .appendField("悬停");
        this.appendDummyInput()
		     .appendField("秒");
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
    var t = Blockly.Python.valueToCode(this,'hover',Blockly.Python.ORDER_ATOMIC);
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
        this.appendValueInput("revolve")
            .setCheck("Number")
            .appendField("旋转");
        this.appendDummyInput()
		    .appendField("度");
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
    var r = Blockly.Python.valueToCode(this,'revolve',Blockly.Python.ORDER_ATOMIC);
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
             ]), "direction");
        this.appendValueInput("distances")
            .setCheck("Number")
            .appendField("飞");
            this.appendDummyInput()
		    .appendField("cm");
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
    var s = Blockly.Python.valueToCode(this,'distances',Blockly.Python.ORDER_ATOMIC);
    var d = block.getFieldValue('direction');
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