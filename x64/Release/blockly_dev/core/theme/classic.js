/**
 * @license
 * Copyright 2018 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview Classic theme.
 * Contains multi-coloured border to create shadow effect.
 */
'use strict';

goog.provide('Blockly.Themes.Classic');

goog.require('Blockly.Theme');


// Temporary holding object.
Blockly.Themes.Classic = {};

Blockly.Themes.Classic.defaultBlockStyles = {
  "colour_blocks": {
    "colourPrimary": "rgb(153, 102, 246)"
  },
  "list_blocks": {
    "colourPrimary": "rgb(246, 213, 0)"
  },
  "logic_blocks": {
    "colourPrimary": "rgb(255, 102, 128)"
  },
  "loop_blocks": {
    "colourPrimary": "rgb(59, 140, 255)"
  },
  "math_blocks": {
    "colourPrimary": "rgb(37, 203, 50)",
    // "colourSecondary":""
  },
  "procedure_blocks": {
    "colourPrimary": "rgb(246, 82, 82)"
  },
  "text_blocks": {
    "colourPrimary": "rgb(18, 179, 246)"
  },
  "variable_blocks": {
    "colourPrimary": "rgb(68, 212, 132)"
  },
  "variable_dynamic_blocks": {
    "colourPrimary": "310"
  },
  "hat_blocks": {
    "colourPrimary": "330",
    "hat": "cap"
  }
};

Blockly.Themes.Classic.categoryStyles = {
  "colour_category": {
    "colour": "20"
  },
  "list_category": {
    "colour": "260"
  },
  "logic_category": {
    "colour": "210"
  },
  "loop_category": {
    "colour": "120"
  },
  "math_category": {
    "colour": "230"
  },
  "procedure_category": {
    "colour": "290"
  },
  "text_category": {
    "colour": "160"
  },
  "variable_category": {
    "colour": "330"
  },
  "variable_dynamic_category": {
    "colour": "310"
  }
};

Blockly.Themes.Classic =
    new Blockly.Theme('classic', Blockly.Themes.Classic.defaultBlockStyles,
        Blockly.Themes.Classic.categoryStyles);
