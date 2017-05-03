/* Copyright (c) 2017. All rights reserved. */

'use strict';

var uniquePositionCLib;

try {
  uniquePositionCLib =  require('../build/Release/UniquePosition');
} catch (err) {
  if (err.code === 'MODULE_NOT_FOUND') {
    uniquePositionCLib = require('../build/Debug/UniquePosition');
  } else {
    throw err;
  }
}

module.exports = uniquePositionCLib;
