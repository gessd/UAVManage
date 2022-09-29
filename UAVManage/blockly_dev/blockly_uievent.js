'use strict';
Blockly.fireUiEventNow = function (a, b) {
    var c = Blockly.fireUiEvent.DB_[b];
    if (c) {
        var d = c.indexOf(a);
        -1 != d && c.splice(d, 1)
    }
    if (document.createEvent)c = document.createEvent("UIEvents"), c.initEvent(b, !0, !0), a.dispatchEvent(c); else if (document.createEventObject)c = document.createEventObject(), a.fireEvent("on" + b, c); else throw"FireEvent: No event creation mechanism.";
};
Blockly.fireUiEvent = function (a, b) {
    var c = Blockly.fireUiEvent.DB_[b];
    if (c) {
        if (-1 != c.indexOf(a))return;
        c.push(a)
    } else Blockly.fireUiEvent.DB_[b] = [a];
    setTimeout(function () {
        Blockly.fireUiEventNow(a, b)
    }, 0)
};
Blockly.fireUiEvent.DB_ = {};