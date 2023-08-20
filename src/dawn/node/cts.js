'use strict';

const { create, globals } = require('./dawn.node');

Object.assign(globalThis, globals);

module.exports = { create };
