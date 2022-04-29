SKIP: FAILED https://crbug.com/tint/1522

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:46:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:60:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:62:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:105:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:119:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:121:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:164:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:178:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:180:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:223:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:237:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:239:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:282:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:296:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:298:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:341:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:355:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:357:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:400:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:414:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:416:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:459:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:473:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:475:9 warning: code is unreachable
        return;
        ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:518:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:532:11 warning: code is unreachable
          return;
          ^^^^^^

vk-gl-cts/graphicsfuzz/spv-load-from-frag-color/1.wgsl:534:9 warning: code is unreachable
        return;
        ^^^^^^

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  BST tree[10] = (BST[10])0;
  int x_360 = 0;
  int x_62_phi = 0;
  bool x_90_phi = false;
  int x_357_phi = 0;
  int x_360_phi = 0;
  int x_362_phi = 0;
  const BST tint_symbol_1 = {9, -1, -1};
  tree[0] = tint_symbol_1;
  0u;
  do {
    x_62_phi = 0;
    [loop] while (true) {
      int x_88 = 0;
      int x_80 = 0;
      int x_63 = 0;
      int x_63_phi = 0;
      const int x_62 = x_62_phi;
      x_90_phi = false;
      if ((x_62 <= 1)) {
      } else {
        break;
      }
      const int x_69 = tree[x_62].data;
      if ((5 <= x_69)) {
        const int x_82_save = x_62;
        const int x_83 = tree[x_82_save].leftIndex;
        if ((x_83 == -1)) {
          tree[x_82_save].leftIndex = 1;
          const BST tint_symbol_2 = {5, -1, -1};
          tree[1] = tint_symbol_2;
          x_90_phi = true;
          break;
        } else {
          x_88 = tree[x_82_save].leftIndex;
          x_63_phi = x_88;
          {
            x_63 = x_63_phi;
            x_62_phi = x_63;
          }
          continue;
        }
        return;
      } else {
        const int x_74_save = x_62;
        const int x_75 = tree[x_74_save].rightIndex;
        if ((x_75 == -1)) {
          tree[x_74_save].rightIndex = 1;
          const BST tint_symbol_3 = {5, -1, -1};
          tree[1] = tint_symbol_3;
          x_90_phi = true;
          break;
        } else {
          x_80 = tree[x_74_save].rightIndex;
          x_63_phi = x_80;
          {
            x_63 = x_63_phi;
            x_62_phi = x_63;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_63 = x_63_phi;
        x_62_phi = x_63;
      }
    }
    if (x_90_phi) {
      break;
    }
  } while (false);
  int x_95_phi = 0;
  bool x_123_phi = false;
  0u;
  do {
    x_95_phi = 0;
    [loop] while (true) {
      int x_121 = 0;
      int x_113 = 0;
      int x_96 = 0;
      int x_96_phi = 0;
      const int x_95 = x_95_phi;
      x_123_phi = false;
      if ((x_95 <= 2)) {
      } else {
        break;
      }
      const int x_102 = tree[x_95].data;
      if ((12 <= x_102)) {
        const int x_115_save = x_95;
        const int x_116 = tree[x_115_save].leftIndex;
        if ((x_116 == -1)) {
          tree[x_115_save].leftIndex = 2;
          const BST tint_symbol_4 = {12, -1, -1};
          tree[2] = tint_symbol_4;
          x_123_phi = true;
          break;
        } else {
          x_121 = tree[x_115_save].leftIndex;
          x_96_phi = x_121;
          {
            x_96 = x_96_phi;
            x_95_phi = x_96;
          }
          continue;
        }
        return;
      } else {
        const int x_107_save = x_95;
        const int x_108 = tree[x_107_save].rightIndex;
        if ((x_108 == -1)) {
          tree[x_107_save].rightIndex = 2;
          const BST tint_symbol_5 = {12, -1, -1};
          tree[2] = tint_symbol_5;
          x_123_phi = true;
          break;
        } else {
          x_113 = tree[x_107_save].rightIndex;
          x_96_phi = x_113;
          {
            x_96 = x_96_phi;
            x_95_phi = x_96;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_96 = x_96_phi;
        x_95_phi = x_96;
      }
    }
    if (x_123_phi) {
      break;
    }
  } while (false);
  int x_128_phi = 0;
  bool x_156_phi = false;
  0u;
  do {
    x_128_phi = 0;
    [loop] while (true) {
      int x_154 = 0;
      int x_146 = 0;
      int x_129 = 0;
      int x_129_phi = 0;
      const int x_128 = x_128_phi;
      x_156_phi = false;
      if ((x_128 <= 3)) {
      } else {
        break;
      }
      const int x_135 = tree[x_128].data;
      if ((15 <= x_135)) {
        const int x_148_save = x_128;
        const int x_149 = tree[x_148_save].leftIndex;
        if ((x_149 == -1)) {
          tree[x_148_save].leftIndex = 3;
          const BST tint_symbol_6 = {15, -1, -1};
          tree[3] = tint_symbol_6;
          x_156_phi = true;
          break;
        } else {
          x_154 = tree[x_148_save].leftIndex;
          x_129_phi = x_154;
          {
            x_129 = x_129_phi;
            x_128_phi = x_129;
          }
          continue;
        }
        return;
      } else {
        const int x_140_save = x_128;
        const int x_141 = tree[x_140_save].rightIndex;
        if ((x_141 == -1)) {
          tree[x_140_save].rightIndex = 3;
          const BST tint_symbol_7 = {15, -1, -1};
          tree[3] = tint_symbol_7;
          x_156_phi = true;
          break;
        } else {
          x_146 = tree[x_140_save].rightIndex;
          x_129_phi = x_146;
          {
            x_129 = x_129_phi;
            x_128_phi = x_129;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_129 = x_129_phi;
        x_128_phi = x_129;
      }
    }
    if (x_156_phi) {
      break;
    }
  } while (false);
  int x_161_phi = 0;
  bool x_189_phi = false;
  0u;
  do {
    x_161_phi = 0;
    [loop] while (true) {
      int x_187 = 0;
      int x_179 = 0;
      int x_162 = 0;
      int x_162_phi = 0;
      const int x_161 = x_161_phi;
      x_189_phi = false;
      if ((x_161 <= 4)) {
      } else {
        break;
      }
      const int x_168 = tree[x_161].data;
      if ((7 <= x_168)) {
        const int x_181_save = x_161;
        const int x_182 = tree[x_181_save].leftIndex;
        if ((x_182 == -1)) {
          tree[x_181_save].leftIndex = 4;
          const BST tint_symbol_8 = {7, -1, -1};
          tree[4] = tint_symbol_8;
          x_189_phi = true;
          break;
        } else {
          x_187 = tree[x_181_save].leftIndex;
          x_162_phi = x_187;
          {
            x_162 = x_162_phi;
            x_161_phi = x_162;
          }
          continue;
        }
        return;
      } else {
        const int x_173_save = x_161;
        const int x_174 = tree[x_173_save].rightIndex;
        if ((x_174 == -1)) {
          tree[x_173_save].rightIndex = 4;
          const BST tint_symbol_9 = {7, -1, -1};
          tree[4] = tint_symbol_9;
          x_189_phi = true;
          break;
        } else {
          x_179 = tree[x_173_save].rightIndex;
          x_162_phi = x_179;
          {
            x_162 = x_162_phi;
            x_161_phi = x_162;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_162 = x_162_phi;
        x_161_phi = x_162;
      }
    }
    if (x_189_phi) {
      break;
    }
  } while (false);
  int x_194_phi = 0;
  bool x_222_phi = false;
  0u;
  do {
    x_194_phi = 0;
    [loop] while (true) {
      int x_220 = 0;
      int x_212 = 0;
      int x_195 = 0;
      int x_195_phi = 0;
      const int x_194 = x_194_phi;
      x_222_phi = false;
      if ((x_194 <= 5)) {
      } else {
        break;
      }
      const int x_201 = tree[x_194].data;
      if ((8 <= x_201)) {
        const int x_214_save = x_194;
        const int x_215 = tree[x_214_save].leftIndex;
        if ((x_215 == -1)) {
          tree[x_214_save].leftIndex = 5;
          const BST tint_symbol_10 = {8, -1, -1};
          tree[5] = tint_symbol_10;
          x_222_phi = true;
          break;
        } else {
          x_220 = tree[x_214_save].leftIndex;
          x_195_phi = x_220;
          {
            x_195 = x_195_phi;
            x_194_phi = x_195;
          }
          continue;
        }
        return;
      } else {
        const int x_206_save = x_194;
        const int x_207 = tree[x_206_save].rightIndex;
        if ((x_207 == -1)) {
          tree[x_206_save].rightIndex = 5;
          const BST tint_symbol_11 = {8, -1, -1};
          tree[5] = tint_symbol_11;
          x_222_phi = true;
          break;
        } else {
          x_212 = tree[x_206_save].rightIndex;
          x_195_phi = x_212;
          {
            x_195 = x_195_phi;
            x_194_phi = x_195;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_195 = x_195_phi;
        x_194_phi = x_195;
      }
    }
    if (x_222_phi) {
      break;
    }
  } while (false);
  int x_227_phi = 0;
  bool x_255_phi = false;
  0u;
  do {
    x_227_phi = 0;
    [loop] while (true) {
      int x_253 = 0;
      int x_245 = 0;
      int x_228 = 0;
      int x_228_phi = 0;
      const int x_227 = x_227_phi;
      x_255_phi = false;
      if ((x_227 <= 6)) {
      } else {
        break;
      }
      const int x_234 = tree[x_227].data;
      if ((2 <= x_234)) {
        const int x_247_save = x_227;
        const int x_248 = tree[x_247_save].leftIndex;
        if ((x_248 == -1)) {
          tree[x_247_save].leftIndex = 6;
          const BST tint_symbol_12 = {2, -1, -1};
          tree[6] = tint_symbol_12;
          x_255_phi = true;
          break;
        } else {
          x_253 = tree[x_247_save].leftIndex;
          x_228_phi = x_253;
          {
            x_228 = x_228_phi;
            x_227_phi = x_228;
          }
          continue;
        }
        return;
      } else {
        const int x_239_save = x_227;
        const int x_240 = tree[x_239_save].rightIndex;
        if ((x_240 == -1)) {
          tree[x_239_save].rightIndex = 6;
          const BST tint_symbol_13 = {2, -1, -1};
          tree[6] = tint_symbol_13;
          x_255_phi = true;
          break;
        } else {
          x_245 = tree[x_239_save].rightIndex;
          x_228_phi = x_245;
          {
            x_228 = x_228_phi;
            x_227_phi = x_228;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_228 = x_228_phi;
        x_227_phi = x_228;
      }
    }
    if (x_255_phi) {
      break;
    }
  } while (false);
  int x_260_phi = 0;
  bool x_288_phi = false;
  0u;
  do {
    x_260_phi = 0;
    [loop] while (true) {
      int x_286 = 0;
      int x_278 = 0;
      int x_261 = 0;
      int x_261_phi = 0;
      const int x_260 = x_260_phi;
      x_288_phi = false;
      if ((x_260 <= 7)) {
      } else {
        break;
      }
      const int x_267 = tree[x_260].data;
      if ((6 <= x_267)) {
        const int x_280_save = x_260;
        const int x_281 = tree[x_280_save].leftIndex;
        if ((x_281 == -1)) {
          tree[x_280_save].leftIndex = 7;
          const BST tint_symbol_14 = {6, -1, -1};
          tree[7] = tint_symbol_14;
          x_288_phi = true;
          break;
        } else {
          x_286 = tree[x_280_save].leftIndex;
          x_261_phi = x_286;
          {
            x_261 = x_261_phi;
            x_260_phi = x_261;
          }
          continue;
        }
        return;
      } else {
        const int x_272_save = x_260;
        const int x_273 = tree[x_272_save].rightIndex;
        if ((x_273 == -1)) {
          tree[x_272_save].rightIndex = 7;
          const BST tint_symbol_15 = {6, -1, -1};
          tree[7] = tint_symbol_15;
          x_288_phi = true;
          break;
        } else {
          x_278 = tree[x_272_save].rightIndex;
          x_261_phi = x_278;
          {
            x_261 = x_261_phi;
            x_260_phi = x_261;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_261 = x_261_phi;
        x_260_phi = x_261;
      }
    }
    if (x_288_phi) {
      break;
    }
  } while (false);
  int x_293_phi = 0;
  bool x_321_phi = false;
  0u;
  do {
    x_293_phi = 0;
    [loop] while (true) {
      int x_319 = 0;
      int x_311 = 0;
      int x_294 = 0;
      int x_294_phi = 0;
      const int x_293 = x_293_phi;
      x_321_phi = false;
      if ((x_293 <= 8)) {
      } else {
        break;
      }
      const int x_300 = tree[x_293].data;
      if ((17 <= x_300)) {
        const int x_313_save = x_293;
        const int x_314 = tree[x_313_save].leftIndex;
        if ((x_314 == -1)) {
          tree[x_313_save].leftIndex = 8;
          const BST tint_symbol_16 = {17, -1, -1};
          tree[8] = tint_symbol_16;
          x_321_phi = true;
          break;
        } else {
          x_319 = tree[x_313_save].leftIndex;
          x_294_phi = x_319;
          {
            x_294 = x_294_phi;
            x_293_phi = x_294;
          }
          continue;
        }
        return;
      } else {
        const int x_305_save = x_293;
        const int x_306 = tree[x_305_save].rightIndex;
        if ((x_306 == -1)) {
          tree[x_305_save].rightIndex = 8;
          const BST tint_symbol_17 = {17, -1, -1};
          tree[8] = tint_symbol_17;
          x_321_phi = true;
          break;
        } else {
          x_311 = tree[x_305_save].rightIndex;
          x_294_phi = x_311;
          {
            x_294 = x_294_phi;
            x_293_phi = x_294;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_294 = x_294_phi;
        x_293_phi = x_294;
      }
    }
    if (x_321_phi) {
      break;
    }
  } while (false);
  int x_326_phi = 0;
  bool x_354_phi = false;
  0u;
  do {
    x_326_phi = 0;
    [loop] while (true) {
      int x_352 = 0;
      int x_344 = 0;
      int x_327 = 0;
      int x_327_phi = 0;
      const int x_326 = x_326_phi;
      x_354_phi = false;
      if ((x_326 <= 9)) {
      } else {
        break;
      }
      const int x_333 = tree[x_326].data;
      if ((13 <= x_333)) {
        const int x_346_save = x_326;
        const int x_347 = tree[x_346_save].leftIndex;
        if ((x_347 == -1)) {
          tree[x_346_save].leftIndex = 9;
          const BST tint_symbol_18 = {13, -1, -1};
          tree[9] = tint_symbol_18;
          x_354_phi = true;
          break;
        } else {
          x_352 = tree[x_346_save].leftIndex;
          x_327_phi = x_352;
          {
            x_327 = x_327_phi;
            x_326_phi = x_327;
          }
          continue;
        }
        return;
      } else {
        const int x_338_save = x_326;
        const int x_339 = tree[x_338_save].rightIndex;
        if ((x_339 == -1)) {
          tree[x_338_save].rightIndex = 9;
          const BST tint_symbol_19 = {13, -1, -1};
          tree[9] = tint_symbol_19;
          x_354_phi = true;
          break;
        } else {
          x_344 = tree[x_338_save].rightIndex;
          x_327_phi = x_344;
          {
            x_327 = x_327_phi;
            x_326_phi = x_327;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_327 = x_327_phi;
        x_326_phi = x_327;
      }
    }
    if (x_354_phi) {
      break;
    }
  } while (false);
  x_357_phi = 0;
  x_360_phi = 0;
  x_362_phi = 0;
  [loop] while (true) {
    int x_392 = 0;
    int x_402 = 0;
    int x_407 = 0;
    int x_363 = 0;
    int x_358_phi = 0;
    int x_361_phi = 0;
    const int x_357 = x_357_phi;
    x_360 = x_360_phi;
    const int x_362 = x_362_phi;
    const int x_365 = (6 - 15);
    if ((x_362 < 20)) {
    } else {
      break;
    }
    int x_374_phi = 0;
    int x_392_phi = 0;
    bool x_393_phi = false;
    0u;
    do {
      x_374_phi = 0;
      [loop] while (true) {
        const int x_374 = x_374_phi;
        x_392_phi = x_357;
        x_393_phi = false;
        if ((x_374 != -1)) {
        } else {
          break;
        }
        const BST x_381 = tree[x_374];
        const int x_382 = x_381.data;
        const int x_383 = x_381.leftIndex;
        const int x_385 = x_381.rightIndex;
        if ((x_382 == x_362)) {
          x_392_phi = x_362;
          x_393_phi = true;
          break;
        }
        const float x_389 = x_GLF_color[((3u <= 3u) ? 3u : 3u)];
        {
          x_374_phi = (!((x_362 <= x_382)) ? x_385 : x_383);
        }
      }
      x_392 = x_392_phi;
      const bool x_393 = x_393_phi;
      x_358_phi = x_392;
      if (x_393) {
        break;
      }
      x_358_phi = -1;
    } while (false);
    int x_358 = 0;
    int x_401 = 0;
    int x_406 = 0;
    int x_402_phi = 0;
    int x_407_phi = 0;
    x_358 = x_358_phi;
    switch(x_362) {
      case 2:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 12:
      case 13:
      case 15:
      case 17: {
        x_402_phi = x_360;
        if ((x_358 == asint(x_362))) {
          x_401 = asint((x_360 + asint(1)));
          x_402_phi = x_401;
        }
        x_402 = x_402_phi;
        x_361_phi = x_402;
        break;
      }
      default: {
        x_407_phi = x_360;
        if ((x_358 == asint(-1))) {
          x_406 = asint((x_360 + asint(1)));
          x_407_phi = x_406;
        }
        x_407 = x_407_phi;
        x_361_phi = x_407;
        break;
      }
    }
    const int x_361 = x_361_phi;
    {
      x_363 = (x_362 + 1);
      x_357_phi = x_358;
      x_360_phi = x_361;
      x_362_phi = x_363;
    }
  }
  if ((x_360 == asint(20))) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 1.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_20 = {x_GLF_color};
  return tint_symbol_20;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000018520DAA240(82,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(22,12-23): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x0000018520DAA240(148,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(214,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(280,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(346,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(412,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(478,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(544,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(610,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000018520DAA240(664,7-19): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
internal error: compilation aborted unexpectedly

