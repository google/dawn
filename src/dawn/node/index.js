'use strict';

const { create, globals } = require('./dawn.node');

module.exports = {
  ...globals,
  gpu: create(process.env.DAWN_FLAGS?.split(',') || []),
};
