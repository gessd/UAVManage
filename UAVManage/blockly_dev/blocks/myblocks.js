'use strict';

goog.require('Blockly.Blocks');
goog.require('Blockly');

Blockly.Blocks['QzrobotStart'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("开始 ");
        this.setNextStatement(true,["action", "notReachAction", "ReachAction","time"]);
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotStart'] = function(block) {
    var code = 'Start()' + '\n';
    return code;
};

Blockly.Blocks['QzrobotLock'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("上锁 ")
        this.setPreviousStatement(true);

        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotLock'] = function(block) {
    var code = 'Disarm(f1)' + '\n';
    return code;
};

Blockly.Blocks['QzrobotUnLock'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("解锁 ")
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]); //向上只能连接action
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]); //向下只能连接action
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotUnLock'] = function(block) {
    var code = 'Arm(f1)' + '\n';
    return code;
};

Blockly.Blocks['Block_Inittime'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("Start at:")
            .appendField(new Blockly.FieldTextInput("00:01", function(newValue) {
                var reg = RegExp("^[0-5]{0,1}[0-9]{1,1}:[0-5]{1,1}[0-9]{1,1}$");
                if (!reg.test(newValue)) {
                    if(newValue.indexOf(":")!=-1)
                    {
                        var minute=newValue.substr(0,newValue.indexOf(":"));
                        var second=newValue.substr(newValue.indexOf(":")+1);
                        var imin=parseInt(minute);
                        var isec=parseInt(second);
                        if(isec>60)
                        {
                            imin+=Math.floor(isec/60);
                            isec=isec%60;
                        }
                        var smin=imin.toFixed(0).toString();
                        var ssec=isec.toFixed(0).toString();
                        var len = smin.length;
                        if(len<2)
                        {
                            smin='0'+smin;
                        }
                        len=ssec.length;
                        if(len<2)
                        {
                            ssec='0'+ssec;
                        }
                        newValue = smin+':'+ssec;
                    }

                }
                return newValue;

            }), "timeParam")
            .appendField(new Blockly.FieldColour("#FFFFFF"), "color");
        this.appendStatementInput("functionIntit")
            .setCheck(null);
        this.setPreviousStatement(true, "time");
        this.setNextStatement(true, "time");
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['Block_Inittime'] = function(block) {
    var text_time = block.getFieldValue('timeParam');
    var statements_functionintit = Blockly.Python.statementToCode(block, 'functionIntit');
    var code = 'inittime(' + text_time + ')' + '\n' + statements_functionintit;
    return code;
};

Blockly.Blocks['QzrobotLand'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("降落 ")
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotLand'] = function(block) {
    var code = 'Land()' + '\n';
    return code; //返回的函数
};

Blockly.Blocks['QzrobotTimerCounter_Start'] = {
    init: function() {
        this.setColour('#FE9A2E');
        this.appendDummyInput()
            .appendField('启动计时器');
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
    }
};

Blockly.Python['QzrobotTimerCounter_Start'] = function(block) {
    var code = 'TimerStart()' + '\n';
    return code;
};

Blockly.Blocks['QzrobotTimerCounter_Get'] = {
    init: function() {
        this.setColour('#FE9A2E');
        this.appendDummyInput()
            .appendField('计时器当前时间');
        this.setOutput(true, null);
    }
};

Blockly.Python['QzrobotTimerCounter_Get'] = function(block) {
    var code = 'GetTimerPassMs()';
    return [code,Blockly.Python.ORDER_ATOMIC];
};

Blockly.Blocks['QzrobotTimerCounter_Clear'] = {
    init: function() {
        this.setColour('#FE9A2E');
        this.appendDummyInput()
            .appendField('计时器归零');
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
    }
};

Blockly.Python['QzrobotTimerCounter_Clear'] = function(block) {
    var code = 'TimerClear()' + '\n';
    return code;
};

Blockly.Blocks['Block_Delay'] = {
    init: function() {
        this.appendDummyInput("")
            .appendField("延时")
            .appendField(new Blockly.FieldDropdown([
                ["ms", "mse"],
                ["s", "sec"],
                ["min", "min"]
            ]), "delay");
        this.appendValueInput("timelength")
            .setCheck("Number");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setInputsInline(true);
        this.setColour('#FE9A2E');
    }
};

Blockly.Python['Block_Delay'] = function(block) {
    var dropdown_delay = block.getFieldValue('delay');
    var dropdown_time = Blockly.Python.valueToCode(this,'timelength',Blockly.Python.ORDER_ATOMIC);
    if(dropdown_delay=="mse")
        var code = 'Delay(' + dropdown_time + ')' + '\n';
    else if(dropdown_delay=="sec")
        var code = 'Delay(' + dropdown_time*1000 + ')' + '\n';
    else if(dropdown_delay=="min")
        var code = 'Delay(' + dropdown_time*60000 + ')' + '\n';
    
    return code;
};

Blockly.Blocks['Fly_time'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("Start at:")
            .appendField(new Blockly.FieldTextInput("00:01", function(newValue) {
                var reg = RegExp("^[0-5]{0,1}[0-9]{1,1}:[0-5]{1,1}[0-9]{1,1}$");
                if (!reg.test(newValue)) {
                    if(newValue.indexOf(":")!=-1){
                        var minute=newValue.substr(0,newValue.indexOf(":"));
                        var second=newValue.substr(newValue.indexOf(":")+1);
                        var imin=parseInt(minute);
                        var isec=parseInt(second);
                        if(isec>60){
                            imin+=Math.floor(isec/60);
                            isec=isec%60;
                        } 
                        var smin=imin.toFixed(0).toString();
                        var ssec=isec.toFixed(0).toString();
                        var len = smin.length;
                        if(len<2){
                            smin='0'+smin;
                        }
                        len=ssec.length;
                        if(len<2){
                            ssec='0'+ssec;
                        }
                        newValue = smin+':'+ssec;
                    }
                }
                return newValue;
            }), "flytime");
        this.appendStatementInput("interiorfunction").setCheck(null);
        this.setPreviousStatement(true, "time");
        this.setNextStatement(true, "time");
        this.setColour('#FF6680');
    }
};
Blockly.Python['Fly_time'] = function(block) {
    var tt = block.getFieldValue('flytime');
    var ff = Blockly.Python.statementToCode(block, 'interiorfunction');
    var code = 'FlyTime("' + tt + '")' + '\n'+ff;
    return code;
};

Blockly.Blocks['Fly_StartLocation'] = {
    init: function() {
        this.appendValueInput("x")
            .setCheck("Number")
            .appendField("初始位置 X:");
        this.appendValueInput("y")
            .setCheck("Number")
            .appendField("Y:");
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#FF6680');
        this.setTooltip('');
        this.setInputsInline(true);
    }
};

Blockly.Python['Fly_StartLocation'] = function(block) {
    //起飞至
    var x = Blockly.Python.valueToCode(this,'x',Blockly.Python.ORDER_ATOMIC);
    var y = Blockly.Python.valueToCode(this,'y',Blockly.Python.ORDER_ATOMIC);
    var code = 'SetStartLocation('+ x + ','+y + ')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotTakeOff'] = {
    init: function() {
        this.appendValueInput("height")
            .setCheck("Number")
            .appendField("起飞至");
        this.appendDummyInput()
            .appendField("cm高度");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#FF6680');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotTakeOff'] = function(block) {
    //起飞至
    var alt = Blockly.Python.valueToCode(this,'height',Blockly.Python.ORDER_ATOMIC);
    var code = 'Takeoff('+ alt + ')' + '\n';
    return code;
};

Blockly.Blocks['SpecifiedpeedTakeOff'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("以速度");
        this.appendValueInput("height")
            .setCheck("Number")
            .appendField("起飞至");
        this.appendDummyInput()
            .appendField("cm高度");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['SpecifiedpeedTakeOff'] = function(block) {
    var text_speed = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var text_height = Blockly.Python.valueToCode(this,'height',Blockly.Python.ORDER_ATOMIC);
    var code = 'SpecifiedpeedTakeOff('+text_speed+','+text_height+')' + '\n';
    return code;
};

Blockly.Blocks['SetFlyLocation'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行");
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

Blockly.Python['SetFlyLocation'] = function(block) {
    var px = Blockly.Python.valueToCode(this,'coordinateX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'coordinateY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'coordinateZ',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyTo(' + px + ',' + py + ',' + pz +')' + '\n';
    return code;
};

Blockly.Blocks['SetFlyLocationAndStay'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("飞行");
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
        this.appendValueInput("stay")
            .setCheck("Number")
            .appendField("悬停");
        this.appendDummyInput()
		     .appendField("秒");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['SetFlyLocationAndStay'] = function(block) {
    var px = Blockly.Python.valueToCode(this,'coordinateX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'coordinateY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'coordinateZ',Blockly.Python.ORDER_ATOMIC);
    var t = Blockly.Python.valueToCode(this,'stay',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyNavpoint(' + px + ',' + py + ',' + pz +',0,0,0,'+t+')' + '\n';
    return code;
};

Blockly.Blocks['SetFlyLocationAndSpeed'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("以速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendDummyInput()
            .appendField("飞行");
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
        this.setInputsInline(true);
    }
};

Blockly.Python['SetFlyLocationAndSpeed'] = function(block) {
    var px = Blockly.Python.valueToCode(this,'coordinateX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'coordinateY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'coordinateZ',Blockly.Python.ORDER_ATOMIC);
    var ps = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyToSpeed(' + px + ',' + py + ',' + pz + ',' + ps +')' + '\n';
    return code;
};

Blockly.Blocks['SetFlySpeed'] = {
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

Blockly.Python['SetFlySpeed'] = function(block) {
    var ps = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var code = 'SetFlySpeed(' + ps +')' + '\n';
    return code;
};

Blockly.Blocks['UAV_Marking_Point'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("添加坐标点");
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
        this.appendValueInput("Hold")
            .setCheck("Number")
            .appendField("停留时间");
        this.appendValueInput("AcceptRadius")
            .setCheck("Number")
            .appendField("接受半径");
        this.appendValueInput("PassRadius")
            .setCheck("Number")
            .appendField("轨迹控制");
        this.appendValueInput("Yaw")
            .setCheck("Number")
            .appendField("偏转角度");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['UAV_Marking_Point'] = function(block) {
    var px = Blockly.Python.valueToCode(this,'coordinateX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'coordinateY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'coordinateZ',Blockly.Python.ORDER_ATOMIC);
    var Hold = Blockly.Python.valueToCode(this,'Hold',Blockly.Python.ORDER_ATOMIC);
    var AcceptRadius = Blockly.Python.valueToCode(this,'AcceptRadius',Blockly.Python.ORDER_ATOMIC);
    var PassRadius = Blockly.Python.valueToCode(this,'PassRadius',Blockly.Python.ORDER_ATOMIC);
    var Yaw = Blockly.Python.valueToCode(this,'Yaw',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyNavpoint(' + px + ',' + py + ',' + pz + ',' + Hold + ',' + AcceptRadius + ',' + PassRadius + ',' + Yaw +')' + '\n';
    return code;
};

Blockly.Blocks['Fly_stay'] = {
    init: function() {
        this.appendValueInput("stay")
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

Blockly.Python['Fly_stay'] = function(block) {
    var t = Blockly.Python.valueToCode(this,'stay',Blockly.Python.ORDER_ATOMIC);
    var code = 'FlyHover('+t+')' + '\n';
    return code; 
};

Blockly.Blocks['Fly_revolve'] = {
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

Blockly.Python['Fly_revolve'] = function(block) {
    var r = Blockly.Python.valueToCode(this,'revolve',Blockly.Python.ORDER_ATOMIC);
    var d = block.getFieldValue('direction');
    if("right" == d){
        return 'FlyRevolve('+r+')' + '\n';
    } else if("left" == d){
        return 'FlyRevolve('+-r+')' + '\n';
    }
};

Blockly.Blocks['Fly_moveto'] = {
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

Blockly.Python['Fly_moveto'] = function(block) {
    var s = Blockly.Python.valueToCode(this,'distances',Blockly.Python.ORDER_ATOMIC);
    var d = block.getFieldValue('direction');
    return 'FlyMoveto('+d+','+s+')\n';
};

Blockly.Blocks['Fly_LedMode'] = {
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

Blockly.Python['Fly_LedMode'] = function(block) {
    var m = block.getFieldValue('mode');
    return 'SetLedMode('+m+')\n';
};

Blockly.Blocks['UAV_MoveToPoint'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("直线移至 ")
            .appendField("坐标点")
            .appendField(new Blockly.FieldTextInput("a"), "point");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};
Blockly.Python['UAV_MoveToPoint'] = function(block) {
    var point = block.getFieldValue('point');
    var code = 'Move2Marker(f1,' + point + ')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotHover'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("悬停 ");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('http://www.example.com/');
    }
};

Blockly.Python['QzrobotHover'] = function(block) {
    //悬停
    var code = 'Hover(f1)' + '\n';
    return code; //返回的函数
};

Blockly.Blocks['QzrobotEmergencyHover'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("紧急悬停 ");
        this.setPreviousStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true,["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
        this.setHelpUrl('http://www.example.com/');
    }
};

Blockly.Python['QzrobotEmergencyHover'] = function(block) {
    var getvalue = block.getFieldValue('land');
    var code = 'Hover(' + getvalue + ')' + '\n';
    return code; //返回的函数
};

Blockly.Blocks['QzrobotSpecifiedSpeed'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("以速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendValueInput("flightdistance")
            .setCheck("Number")
            .appendField("飞行距离");
        this.appendDummyInput()
            .appendField("cm");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotSpecifiedSpeed'] = function(block) {
    var Gspeed = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var jl = Blockly.Python.valueToCode(this,'flightdistance',Blockly.Python.ORDER_ATOMIC);
    var code = 'Mspned('+Gspeed+','+jl+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotHorizontalSpeed'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("以速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendDummyInput()
            .appendField("水平方向")
            .appendField(new Blockly.FieldDropdown([
                ["前", "front"],
                ["后", "after"],
                ["左", "left"],
                ["右", "right"]
            ]), "direction");
        this.appendValueInput("flightdistance")
            .setCheck("Number")
            .appendField("飞行距离");
        this.appendDummyInput()
            .appendField("cm");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotHorizontalSpeed'] = function(block) {
    var VHS = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var dropdown_direction = block.getFieldValue('direction');
    var JLX = Blockly.Python.valueToCode(this,'flightdistance',Blockly.Python.ORDER_ATOMIC);
    var code = 'Dctmove('+VHS+','+dropdown_direction+','+JLX+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotVerticalSpeed'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("以速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendDummyInput()
            .appendField("垂直方向")
            .appendField(new Blockly.FieldDropdown([
                ["上", "up"],
                ["下", "down"],
            ]), "direction")
        this.appendValueInput("flightdistance")
            .setCheck("Number")
            .appendField("飞行距离");
        this.appendDummyInput()
            .appendField("cm");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotVerticalSpeed'] = function(block) {
    var hspeed = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var dropdown_direction = block.getFieldValue('direction');
    var jl = Blockly.Python.valueToCode(this,'flightdistance',Blockly.Python.ORDER_ATOMIC);
    var code = 'Moveud('+hspeed+','+dropdown_direction+','+jl+')' + '\n';
    return code;
};

//向左转向右转
Blockly.Blocks['QzrobotAngularVelocity'] = {
    init: function() {
        this.appendValueInput("velocity")
            .setCheck("Number")
            .appendField("以角速度");
        this.appendDummyInput()
            .appendField("°/s");
        this.appendDummyInput()
            .appendField("向")
            .appendField(new Blockly.FieldDropdown([
                ["左", "left"],
                ["右", "right"]
            ]), "turnDirection");
        this.appendValueInput("angle")
            .setCheck("Number")
            .appendField("旋转");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotAngularVelocity'] = function(block) {
    var velocity =  Blockly.Python.valueToCode(this,'velocity',Blockly.Python.ORDER_ATOMIC);
    var turnDirection = block.getFieldValue('turnDirection');
    var angle = Blockly.Python.valueToCode(this,'angle',Blockly.Python.ORDER_ATOMIC);
    var code = 'Yawlr('+velocity+','+turnDirection+','+angle+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotMove'] = {
    init: function() {
        this.appendValueInput("directionX")
            .setCheck("Number")
            .appendField("X方向移动");
        this.appendDummyInput()
            .appendField("cm");
        this.appendValueInput("directionY")
            .setCheck("Number")
            .appendField("Y方向移动");
        this.appendDummyInput()
            .appendField("cm");
        this.appendValueInput("directionZ")
            .setCheck("Number")
            .appendField("Z方向移动");
        this.appendDummyInput()
            .appendField("cm");
        this.setInputsInline(true);
        this.setPreviousStatement(true, "notReachAction");
        this.setNextStatement(true, "notReachAction");
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotMove'] = function(block) {
    var px = Blockly.Python.valueToCode(this,'directionX',Blockly.Python.ORDER_ATOMIC);
    var py = Blockly.Python.valueToCode(this,'directionY',Blockly.Python.ORDER_ATOMIC);
    var pz = Blockly.Python.valueToCode(this,'directionZ',Blockly.Python.ORDER_ATOMIC);
    var code = 'MoveDelta('+px+','+py+','+pz+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotHorizontalSpeedcp'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("水平速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendValueInput("AH")
            .setCheck("Number")
            .appendField("水平加速度");
        this.appendDummyInput()
            .appendField("cm/s²");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotHorizontalSpeedcp'] = function(block) {
    var hspeed = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
    var ha = Blockly.Python.valueToCode(this,'AH',Blockly.Python.ORDER_ATOMIC);
    var code = 'VelAccXY('+hspeed+','+ha+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotVerticalSpeedcp'] = {
    init: function() {
        this.appendValueInput("speed")
            .setCheck("Number")
            .appendField("垂直速度");
        this.appendDummyInput()
            .appendField("cm/s");
        this.appendValueInput("AV")
            .setCheck("Number")
            .appendField("垂直加速度");
        this.appendDummyInput()
            .appendField("cm/s²");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotVerticalSpeedcp'] = function(block) {
    var hspeed = Blockly.Python.valueToCode(this,'speed',Blockly.Python.ORDER_ATOMIC);
     var ha = Blockly.Python.valueToCode(this,'AV',Blockly.Python.ORDER_ATOMIC);
    var code = 'MaxVelZ(f1,' + hspeed + ',' + ha + ')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotDirection'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("无人机面向")
            .appendField(new Blockly.FieldDropdown([
                ["正东", "east"],
                ["正南", "south"],
                ["正西", "west"],
                ["正北", "north"]
            ]), "eswn");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#3B8CFF');
        this.setTooltip('');
    }
};

Blockly.Python['QzrobotDirection'] = function(block) {
    var direction = block.getFieldValue('eswn');
    var code = 'Desnw('+direction+')' + '\n';
    return code;
};

/**
 * 检测
 * @type {{init: Blockly.Blocks.UAV_DetectedData.init}}
 */
Blockly.Blocks['UAV_DetectedData'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("距离")
            .appendField(new Blockly.FieldDropdown([["无人机1","f1"]]), "flight")
            .appendField(new Blockly.FieldDropdown([["上方","up"], ["下方","down"], ["前方","front"],["后方","after"]]), "direction");
        this.appendValueInput("detecteddata")
            .setCheck("Number");
        this.appendDummyInput()
            .appendField("cm发现")
            .appendField(new Blockly.FieldDropdown([["环境要素1","env1"]]), "envelements");
        this.setOutput(true, null);
        this.setColour('#12B3F6');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['UAV_DetectedData'] = function(block) {
    var dropdown_direction = block.getFieldValue('direction');
    var text_detecteddata = Blockly.Python.valueToCode(this,'detecteddata',Blockly.Python.ORDER_ATOMIC);
    var dropdown_envelements = block.getFieldValue('envelements');
    var code = 'detectedOne('+dropdown_direction+','+text_detecteddata+','+dropdown_envelements+')' + '\n';
    return [code,Blockly.Python.ORDER_ATOMIC];
};

Blockly.Blocks['UAV_Dexecute_Drone'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("当")
            .appendField(new Blockly.FieldDropdown([["无人机1","f1"]]), "flight")
            .appendField("检测与")
            .appendField(new Blockly.FieldDropdown([["上","up"], ["下","down"], ["前","front"], ["后","after"]]), "direction")
            .appendField(new Blockly.FieldDropdown([["环境要素1","env1"]]), "envelements")
            .appendField("距离小于");
        this.appendValueInput("detecteddata")
            .setCheck("Number");
        this.appendDummyInput()
            .appendField("cm时，则执行");
        this.appendStatementInput("execute")
            .setCheck(null);
        this.setOutput(true, null);
        this.setColour('#12B3F6');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['UAV_Dexecute_Drone'] = function(block) {
    var dropdown_direction = block.getFieldValue('direction');
    var dropdown_envelements = block.getFieldValue('envelements');
    var text_detecteddata = Blockly.Python.valueToCode(this,'detecteddata',Blockly.Python.ORDER_ATOMIC);
    var statements_execute = Blockly.Python.statementToCode(block, 'execute');
    var code = 'dexecutedrone('+dropdown_direction+','+dropdown_envelements+','+text_detecteddata+')' + '\n' + statements_execute;
    return [code,Blockly.Python.ORDER_ATOMIC];
};

Blockly.Blocks['UAV_FlightPOSXYZ'] = {
    init: function() {
        this.appendDummyInput()
            .appendField(new Blockly.FieldDropdown([["无人机1","f1"]]), "flight")
            .appendField("的当前坐标值");
        this.setInputsInline(true);
        this.setOutput(true, null);
        this.setColour('#12B3F6');
        this.setTooltip('');
    }
};

Blockly.Python['UAV_FlightPOSXYZ'] = function(block) {
    var flight = block.getFieldValue('flight');
    var code = 'readxyz('+flight+')' + '\n';
    return [code,Blockly.Python.ORDER_ATOMIC];
};

Blockly.Blocks['UAV_SetUnityPoint'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("在")
            .appendField(new Blockly.FieldDropdown([["无人机1","f1"]]),"flight")
            .appendField("的当前位置设置标记点并命名为")
            .appendField(new Blockly.FieldTextInput("a"), "name");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#12B3F6');
        this.setTooltip('');
        this.setHelpUrl('http://www.example.com/');
    }
};
Blockly.Python['UAV_SetUnityPoint'] = function(block) {
    var name = block.getFieldValue('name');
    var flight = block.getFieldValue('flight');
    var code = 'SetUnityPoint('+flight+',' + name + ')' + '\n';
    return code;
};

Blockly.Blocks['UAV_Drone_Flying_Style'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("检测")
        .appendField(new Blockly.FieldDropdown([["无人机1","f1"], ["无人机2","f2"], ["无人机3","f3"], ["无人机4","f4"], ["无人机5","f5"], ["无人机6","f6"], ["无人机7","f7"], ["无人机8","f8"]]), "flight")
        .appendField(new Blockly.FieldLabelSerializable("的姿态（传感器角度、速度、偏航、俯仰、翻滚）"), "attitude");
    this.setOutput(true, null);
    this.setColour('#12B3F6');
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Python['UAV_Drone_Flying_Style'] = function(block) {
    var flight = block.getFieldValue('flight');
    var code = 'droneFlyingStyle('+flight+')' + '\n';
    return [code,Blockly.Python.ORDER_ATOMIC];
};

Blockly.Blocks['UAV_Drone_Style'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldLabelSerializable("检测"), "DRONE_STYLE_TITLEHED")
        .appendField(new Blockly.FieldDropdown([["无人机1","f1"]]), "flight")
        .appendField(new Blockly.FieldLabelSerializable("的"), "DRONE_STYLE_TITLEBOY")
        .appendField(new Blockly.FieldDropdown([["前","front"], ["后","after"],["左","left"],["右","right"],["上","up"],["下","down"]]), "direction")
        .appendField("姿态传感器加速度 ");
      this.appendValueInput("AH")
          .setCheck("Number");
      this.appendDummyInput()
        .appendField("cm/s²");
    this.setOutput(true, null);
    this.setColour('#12B3F6');
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Python['UAV_Drone_Style'] = function(block) {
    var flight = block.getFieldValue('flight');
    var direction = block.getFieldValue('direction');
    var ah = Blockly.Python.valueToCode(this,'AH',Blockly.Python.ORDER_ATOMIC);
    var code = 'flyingStyle('+flight+',' + direction + ','+ah+')' + '\n';
    return [code,Blockly.Python.ORDER_ATOMIC];
};

/**
 * 显示
 * @type {{init: Blockly.Blocks.QzrobotAtomicLEDOn.init}}
 */
Blockly.Blocks['QzrobotAtomicLEDOn'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("指定")
            .appendField(new Blockly.FieldDropdown([["1号","led1"], ["2号","led2"], ["3号","led3"],["4号","led4"], ["5号","led5"], ["6号","led6"],["7号","led7"], ["8号","led8"], ["9号","led9"],["10号","led10"], ["11号","led11"], ["12号","led12"]]), "led")
            .appendField("灯")
            .appendField("颜色")
            .appendField(new Blockly.FieldColour("#33cc00"), "color");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#25CB32');
        this.setTooltip('');
        this.setHelpUrl('http://www.example.com/');
    }
};

Blockly.Python['QzrobotAtomicLEDOn'] = function(block) {
    var text_id = block.getFieldValue('led');
    var text_color = block.getFieldValue('color');
    var code = 'SpecifySingle(f1,'+text_id+','+text_color+')'+'\n';
    return code;
};

Blockly.Blocks['QzrobotLed_Colour'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("指定")
            .appendField(new Blockly.FieldDropdown([["1号","led1"], ["2号","led2"], ["3号","led3"],["4号","led4"], ["5号","led5"], ["6号","led6"],["7号","led7"], ["8号","led8"], ["9号","led9"],["10号","led10"], ["11号","led11"], ["12号","led12"]]), "led")
            .appendField("灯")
            .appendField("颜色")
            .appendField(new Blockly.FieldColour("#999999"), "color");
        this.appendValueInput("timelength")
            .setCheck("Number")
            .appendField("持续时长为");
        this.appendDummyInput()
            .appendField("ms");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#25CB32');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['QzrobotLed_Colour'] = function(block) {
    var dropdown_led = block.getFieldValue('led');
    var color1 = block.getFieldValue('color');
    var text_led_color_time = Blockly.Python.valueToCode(this,'timelength',Blockly.Python.ORDER_ATOMIC);
    var code = 'LedColour(f1,'+dropdown_led+','+color1+','+text_led_color_time+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotLed_Colour_Right'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("指定")
            .appendField(new Blockly.FieldDropdown([["1号","led1"], ["2号","led2"], ["3号","led3"],["4号","led4"], ["5号","led5"], ["6号","led6"],["7号","led7"], ["8号","led8"], ["9号","led9"],["10号","led10"], ["11号","led11"], ["12号","led12"]]), "led")
            .appendField("灯")
            .appendField("颜色")
            .appendField(new Blockly.FieldColour("#3366ff"), "color")
            .appendField("亮度为")
            .appendField(new Blockly.FieldDropdown([
                ["100%", "1"],
                ["80%", "0.8"],
                ["60%", "0.6"],
                ["40%", "0.4"],
                ["20%", "0.2"]
            ]), "bright");
        this.appendValueInput("timelength")
            .setCheck("Number")
            .appendField("在");
        this.appendDummyInput()
            .appendField("ms后逐渐变暗");
        this.setInputsInline(true);
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#25CB32');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['QzrobotLed_Colour_Right'] = function(block) {
    var dropdown_led = block.getFieldValue('led');
    var color = block.getFieldValue('color');
    var dropdown_bright = block.getFieldValue('bright');
    var text_led_color_time = Blockly.Python.valueToCode(this,'timelength',Blockly.Python.ORDER_ATOMIC);
    var code = 'LedColorRight(f1,'+dropdown_led+','+color+','+dropdown_bright+','+text_led_color_time+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotLEDSingleBlink'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("报警提示灯")
            .appendField("颜色")
            .appendField(new Blockly.FieldColour("#ff0000"), "color");
        this.appendValueInput("timelength")
            .setCheck("Number")
            .appendField("闪烁");
        this.appendDummyInput()
            .appendField("ms");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#25CB32');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};
Blockly.Python['QzrobotLEDSingleBlink'] = function(block) {
    var text_color1 = block.getFieldValue('color');
    var time = Blockly.Python.valueToCode(this,'timelength',Blockly.Python.ORDER_ATOMIC);
    var code = 'BlinkSingle(f1,'+text_color1+','+time+')' + '\n';
    return code;
};

Blockly.Blocks['QzrobotLEDRgb'] = {
    init: function() {
        this.appendDummyInput()
            .appendField("指定")
            .appendField(new Blockly.FieldDropdown([["1号","led1"], ["2号","led2"], ["3号","led3"],["4号","led4"], ["5号","led5"], ["6号","led6"],["7号","led7"], ["8号","led8"], ["9号","led9"],["10号","led10"], ["11号","led11"], ["12号","led12"]]), "led")
            .appendField("灯");
        this.appendValueInput("rvalue")
            .setCheck("Number")
            .appendField("R值");
        this.appendDummyInput()
            .appendField("");
        this.appendValueInput("gvalue")
            .setCheck("Number")
            .appendField("G值");
        this.appendDummyInput()
            .appendField("");
        this.appendValueInput("bvalue")
            .setCheck("Number")
            .appendField("B值");
        this.appendDummyInput()
            .appendField("");
        this.setPreviousStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setNextStatement(true, ["action", "notReachAction", "ReachAction"]);
        this.setColour('#25CB32');
        this.setTooltip("");
        this.setHelpUrl("");
    }
};

Blockly.Python['QzrobotLEDRgb'] = function(block) {
    var dropdown_led = block.getFieldValue('led');
    var text_rvalue = Blockly.Python.valueToCode(this,'rvalue',Blockly.Python.ORDER_ATOMIC);
    var text_gvalue = Blockly.Python.valueToCode(this,'gvalue',Blockly.Python.ORDER_ATOMIC);
    var text_bvalue = Blockly.Python.valueToCode(this,'bvalue',Blockly.Python.ORDER_ATOMIC);
    var code = 'LedRgb(f1,'+dropdown_led+','+text_rvalue+','+text_gvalue+','+text_bvalue+')' + '\n';
    return code;
};