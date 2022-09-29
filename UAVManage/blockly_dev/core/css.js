/**
 * @license
 * Copyright 2013 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview Inject Blockly's CSS synchronously.
 * @author fraser@google.com (Neil Fraser)
 */
'use strict';

/**
 * @name Blockly.Css
 * @namespace
 */
goog.provide('Blockly.Css');


/**
 * Has CSS already been injected?
 * @type {boolean}
 * @private
 */
Blockly.Css.injected_ = false;

/**
 * Add some CSS to the blob that will be injected later.  Allows optional
 * components such as fields and the toolbox to store separate CSS.
 * The provided array of CSS will be destroyed by this function.
 * @param {!Array.<string>} cssArray Array of CSS strings.
 */
Blockly.Css.register = function(cssArray) {
  if (Blockly.Css.injected_) {
    throw Error('CSS already injected');
  }
  // Concatenate cssArray onto Blockly.Css.CONTENT.
  Array.prototype.push.apply(Blockly.Css.CONTENT, cssArray);
  cssArray.length = 0;  // Garbage collect provided CSS content.
};

/**
 * Inject the CSS into the DOM.  This is preferable over using a regular CSS
 * file since:
 * a) It loads synchronously and doesn't force a redraw later.
 * b) It speeds up loading by not blocking on a separate HTTP transfer.
 * c) The CSS content may be made dynamic depending on init options.
 * @param {boolean} hasCss If false, don't inject CSS
 *     (providing CSS becomes the document's responsibility).
 * @param {string} pathToMedia Path from page to the Blockly media directory.
 */
Blockly.Css.inject = function(hasCss, pathToMedia) {
  // Only inject the CSS once.
  if (Blockly.Css.injected_) {
    return;
  }
  Blockly.Css.injected_ = true;
  var text = Blockly.Css.CONTENT.join('\n');
  Blockly.Css.CONTENT.length = 0;  // Garbage collect CSS content.
  if (!hasCss) {
    return;
  }
  // Strip off any trailing slash (either Unix or Windows).
  var mediaPath = pathToMedia.replace(/[\\/]$/, '');
  text = text.replace(/<<<PATH>>>/g, mediaPath);

  // Inject CSS tag at start of head.
  var cssNode = document.createElement('style');
  cssNode.id = 'blockly-common-style';
  var cssTextNode = document.createTextNode(text);
  cssNode.appendChild(cssTextNode);
  document.head.insertBefore(cssNode, document.head.firstChild);
};

/**
 * Set the cursor to be displayed when over something draggable.
 * See See https://github.com/google/blockly/issues/981 for context.
 * @param {*} _cursor Enum.
 * @deprecated April 2017.
 */
Blockly.Css.setCursor = function(_cursor) {
  console.warn('Deprecated call to Blockly.Css.setCursor. ' +
      'See https://github.com/google/blockly/issues/981 for context');
};

/**
 * Array making up the CSS content for Blockly.
 */
Blockly.Css.CONTENT = [
  /* eslint-disable indent */
  
  '.blocklyDropDownDiv {',
    'position: absolute;',
    'left: 0;',
    'top: 0;',
    'z-index: 1000;',
    'display: none;',
    'border: 1px solid;',
    'border-color: #dadce0;',
    'background-color: #fff;',
    'border-radius: 2px;',
    'padding: 4px;',
    'box-shadow: 0px 0px 3px 1px rgba(0,0,0,.3);',
  '}',

  '.blocklyDropDownDiv.focused {',
    'box-shadow: 0px 0px 6px 1px rgba(0,0,0,.3);',
  '}',

  '.blocklyDropDownContent {',
    'max-height: 300px;', // @todo: spec for maximum height.
    'overflow: auto;',
    'overflow-x: hidden;',
  '}',

  '.blocklyDropDownArrow {',
    'position: absolute;',
    'left: 0;',
    'top: 0;',
    'width: 16px;',
    'height: 16px;',
    'z-index: -1;',
    'background-color: inherit;',
    'border-color: inherit;',
  '}',

  '.blocklyDropDownButton {',
    'display: inline-block;',
    'float: left;',
    'padding: 0;',
    'margin: 4px;',
    'border-radius: 4px;',
    'outline: none;',
    'border: 1px solid;',
    'transition: box-shadow .1s;',
    'cursor: pointer;',
  '}',
  
   /* 28px on the left for icon or checkbox. */
  '.blocklyWidgetDiv .blocklyDropdownMenu .goog-menuitem,',
  '.blocklyDropDownDiv .blocklyDropdownMenu .goog-menuitem {',
    'padding-left: 28px;',
  '}',

  /* BiDi override for the resting state. */
  /* #noflip */
  /* Flip left/right padding for BiDi. */
  '.blocklyWidgetDiv .blocklyDropdownMenu .goog-menuitem.goog-menuitem-rtl,',
  '.blocklyDropDownDiv .blocklyDropdownMenu .goog-menuitem.goog-menuitem-rtl {',  
    'padding-left: 5px;',
    'padding-right: 28px;',
  '}',

  '.blocklyVerticalMarker {',
    'stroke-width: 3px;',
    'fill: rgba(255,255,255,.5);',
    'pointer-events: none',
  '}',

  '.blocklyWidgetDiv .goog-option-selected .goog-menuitem-checkbox,',
  '.blocklyWidgetDiv .goog-option-selected .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-option-selected .goog-menuitem-checkbox,',
  '.blocklyDropDownDiv .goog-option-selected .goog-menuitem-icon {',
    'background: url(<<<PATH>>>/sprites.png) no-repeat -48px -16px;',
  '}',

  '.blocklyWidgetDiv .goog-menu {',
    'background: #fff;',
    'border-color: transparent;',
    'border-style: solid;',
    'border-width: 1px;',
    'cursor: default;',
    'font: normal 13px Arial, sans-serif;',
    'margin: 0;',
    'outline: none;',
    'padding: 4px 0;',
    'position: absolute;',
    'overflow-y: auto;',
    'overflow-x: hidden;',
    'max-height: 100%;',
    'z-index: 20000;',  /* Arbitrary, but some apps depend on it... */
    'box-shadow: 0px 0px 3px 1px rgba(0,0,0,.3);',
  '}',

  '.blocklyWidgetDiv .goog-menu.focused {',
    'box-shadow: 0px 0px 6px 1px rgba(0,0,0,.3);',
  '}',

  '.blocklyDropDownDiv .goog-menu {',
    'cursor: default;',
    'font: normal 13px "Helvetica Neue", Helvetica, sans-serif;',
    'outline: none;',
    'z-index: 20000;',  /* Arbitrary, but some apps depend on it... */
  '}',

  '.blocklyWidgetDiv .goog-menuitem,',
  '.blocklyDropDownDiv .goog-menuitem {',
    'color: #000;',
    'font: normal 13px Arial, sans-serif;',
    'list-style: none;',
    'margin: 0;', /* 7em on the right for shortcut. */
    'min-width: 7em;',
    'border: none;',
    'padding: 6px 15px;',
    'white-space: nowrap;',
    'cursor: pointer;',
  '}',

  /* If a menu doesn't have checkable items or items with icons,
   * remove padding.
   */
  '.blocklyWidgetDiv .goog-menu-nocheckbox .goog-menuitem,',
  '.blocklyWidgetDiv .goog-menu-noicon .goog-menuitem,',
  '.blocklyDropDownDiv .goog-menu-nocheckbox .goog-menuitem,',
  '.blocklyDropDownDiv .goog-menu-noicon .goog-menuitem {',
    'padding-left: 12px;',
  '}',

  '.blocklyWidgetDiv .goog-menuitem-content,',
  '.blocklyDropDownDiv .goog-menuitem-content {',
    'font-family: Arial, sans-serif;',
    'font-size: 13px;',
  '}',

  '.blocklyWidgetDiv .goog-menuitem-content {',
    'color: #000;',
  '}',

  '.blocklyDropDownDiv .goog-menuitem-content {',
    'color: #000;',
  '}',

  /* State: disabled. */
  '.blocklyWidgetDiv .goog-menuitem-disabled,',
  '.blocklyDropDownDiv .goog-menuitem-disabled {',
    'cursor: inherit;',
  '}',

  '.blocklyWidgetDiv .goog-menuitem-disabled .goog-menuitem-content,',
  '.blocklyDropDownDiv .goog-menuitem-disabled .goog-menuitem-content {',
    'color: #ccc !important;',
  '}',

  '.blocklyWidgetDiv .goog-menuitem-disabled .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-menuitem-disabled .goog-menuitem-icon {',
    'opacity: .3;',
    'filter: alpha(opacity=30);',
  '}',

  /* State: hover. */
  '.blocklyWidgetDiv .goog-menuitem-highlight ,',
  '.blocklyDropDownDiv .goog-menuitem-highlight {',
    'background-color: rgba(0,0,0,.1);',
  '}',

  /* State: selected/checked. */
  '.blocklyWidgetDiv .goog-menuitem-checkbox,',
  '.blocklyWidgetDiv .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-menuitem-checkbox,',
  '.blocklyDropDownDiv .goog-menuitem-icon {',
    'background-repeat: no-repeat;',
    'height: 16px;',
    'left: 6px;',
    'position: absolute;',
    'right: auto;',
    'vertical-align: middle;',
    'width: 16px;',
  '}',

  /* BiDi override for the selected/checked state. */
  /* #noflip */
  '.blocklyWidgetDiv .goog-menuitem-rtl .goog-menuitem-checkbox,',
  '.blocklyWidgetDiv .goog-menuitem-rtl .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-menuitem-rtl .goog-menuitem-checkbox,',
  '.blocklyDropDownDiv .goog-menuitem-rtl .goog-menuitem-icon {',
     /* Flip left/right positioning. */
    'left: auto;',
    'right: 6px;',
  '}',

  '.blocklyWidgetDiv .goog-option-selected .goog-menuitem-checkbox,',
  '.blocklyWidgetDiv .goog-option-selected .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-option-selected .goog-menuitem-checkbox,',
  '.blocklyDropDownDiv .goog-option-selected .goog-menuitem-icon {',
    'position: static;', /* Scroll with the menu. */
    'float: left;',
    'margin-left: -24px;',
  '}',

  '.blocklyWidgetDiv .goog-menuitem-rtl .goog-menuitem-checkbox,',
  '.blocklyWidgetDiv .goog-menuitem-rtl .goog-menuitem-icon,',
  '.blocklyDropDownDiv .goog-menuitem-rtl .goog-menuitem-checkbox,',
  '.blocklyDropDownDiv .goog-menuitem-rtl .goog-menuitem-icon {',
    'float: right;',
    'margin-right: -24px;',
  '}',
];
