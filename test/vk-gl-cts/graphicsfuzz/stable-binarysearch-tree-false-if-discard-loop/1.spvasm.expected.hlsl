SKIP: FAILED

warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
warning: code is unreachable
struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  BST tree[10] = (BST[10])0;
  bool x_67 = false;
  bool x_114 = false;
  int x_572 = 0;
  bool x_67_phi = false;
  int x_70_phi = 0;
  bool x_116_phi = false;
  int x_119_phi = 0;
  int x_569_phi = 0;
  int x_572_phi = 0;
  int x_574_phi = 0;
  const BST tint_symbol_4 = {9, -1, -1};
  tree[0] = tint_symbol_4;
  0u;
  do {
    x_67_phi = false;
    x_70_phi = 0;
    [loop] while (true) {
      int x_95 = 0;
      int x_87 = 0;
      bool x_68 = false;
      int x_71 = 0;
      bool x_68_phi = false;
      int x_71_phi = 0;
      x_67 = x_67_phi;
      const int x_70 = x_70_phi;
      x_116_phi = x_67;
      if ((x_70 <= 1)) {
      } else {
        break;
      }
      const int x_76 = tree[x_70].data;
      if ((5 <= x_76)) {
        bool x_114_phi = false;
        const int x_89_save = x_70;
        const int x_90 = tree[x_89_save].leftIndex;
        if ((x_90 == -1)) {
          const float x_97 = asfloat(x_8[0].y);
          const float x_99 = asfloat(x_8[0].x);
          if ((x_97 < x_99)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_89_save].leftIndex = 1;
          const BST tint_symbol_5 = {5, -1, -1};
          tree[1] = tint_symbol_5;
          [loop] while (true) {
            x_114_phi = x_67;
            if ((0 < int(x_97))) {
            } else {
              break;
            }
            x_114_phi = true;
            break;
          }
          x_114 = x_114_phi;
          x_116_phi = x_114;
          if (x_114) {
            break;
          }
        } else {
          x_95 = tree[x_89_save].leftIndex;
          x_68_phi = x_67;
          x_71_phi = x_95;
          {
            x_68 = x_68_phi;
            x_71 = x_71_phi;
            x_67_phi = x_68;
            x_70_phi = x_71;
          }
          continue;
        }
      } else {
        const int x_81_save = x_70;
        const int x_82 = tree[x_81_save].rightIndex;
        if ((x_82 == -1)) {
          tree[x_81_save].rightIndex = 1;
          const BST tint_symbol_6 = {5, -1, -1};
          tree[1] = tint_symbol_6;
          x_116_phi = true;
          break;
        } else {
          x_87 = tree[x_81_save].rightIndex;
          x_68_phi = x_67;
          x_71_phi = x_87;
          {
            x_68 = x_68_phi;
            x_71 = x_71_phi;
            x_67_phi = x_68;
            x_70_phi = x_71;
          }
          continue;
        }
        return;
      }
      x_68_phi = x_114;
      x_71_phi = x_70;
      {
        x_68 = x_68_phi;
        x_71 = x_71_phi;
        x_67_phi = x_68;
        x_70_phi = x_71;
      }
    }
    if (x_116_phi) {
      break;
    }
  } while (false);
  x_119_phi = 0;
  [loop] while (true) {
    bool x_133 = false;
    int x_120 = 0;
    bool x_134_phi = false;
    const int x_119 = x_119_phi;
    const float x_125 = gl_FragCoord.y;
    const bool x_126 = (x_125 < 0.0f);
    x_134_phi = x_126;
    if (!(x_126)) {
      const float x_131 = asfloat(x_8[0].y);
      x_133 = (x_119 != int(x_131));
      x_134_phi = x_133;
    }
    if (x_134_phi) {
    } else {
      break;
    }
    bool x_139 = false;
    bool x_186 = false;
    bool x_139_phi = false;
    int x_142_phi = 0;
    bool x_188_phi = false;
    0u;
    do {
      x_139_phi = false;
      x_142_phi = 0;
      [loop] while (true) {
        int x_167 = 0;
        int x_159 = 0;
        bool x_140 = false;
        int x_143 = 0;
        bool x_140_phi = false;
        int x_143_phi = 0;
        x_139 = x_139_phi;
        const int x_142 = x_142_phi;
        x_188_phi = x_139;
        if ((x_142 <= 2)) {
        } else {
          break;
        }
        const int x_148 = tree[x_142].data;
        if ((12 <= x_148)) {
          bool x_186_phi = false;
          const int x_161_save = x_142;
          const int x_162 = tree[x_161_save].leftIndex;
          if ((x_162 == -1)) {
            const float x_169 = asfloat(x_8[0].y);
            const float x_171 = asfloat(x_8[0].x);
            if ((x_169 < x_171)) {
              [loop] while (true) {
                discard;
              }
              return;
            }
            tree[x_161_save].leftIndex = 2;
            const BST tint_symbol_7 = {12, -1, -1};
            tree[2] = tint_symbol_7;
            [loop] while (true) {
              x_186_phi = x_139;
              if ((0 < int(x_169))) {
              } else {
                break;
              }
              x_186_phi = true;
              break;
            }
            x_186 = x_186_phi;
            x_188_phi = x_186;
            if (x_186) {
              break;
            }
          } else {
            x_167 = tree[x_161_save].leftIndex;
            x_140_phi = x_139;
            x_143_phi = x_167;
            {
              x_140 = x_140_phi;
              x_143 = x_143_phi;
              x_139_phi = x_140;
              x_142_phi = x_143;
            }
            continue;
          }
        } else {
          const int x_153_save = x_142;
          const int x_154 = tree[x_153_save].rightIndex;
          if ((x_154 == -1)) {
            tree[x_153_save].rightIndex = 2;
            const BST tint_symbol_8 = {12, -1, -1};
            tree[2] = tint_symbol_8;
            x_188_phi = true;
            break;
          } else {
            x_159 = tree[x_153_save].rightIndex;
            x_140_phi = x_139;
            x_143_phi = x_159;
            {
              x_140 = x_140_phi;
              x_143 = x_143_phi;
              x_139_phi = x_140;
              x_142_phi = x_143;
            }
            continue;
          }
          return;
        }
        x_140_phi = x_186;
        x_143_phi = x_142;
        {
          x_140 = x_140_phi;
          x_143 = x_143_phi;
          x_139_phi = x_140;
          x_142_phi = x_143;
        }
      }
      if (x_188_phi) {
        break;
      }
    } while (false);
    {
      x_120 = (x_119 + 1);
      x_119_phi = x_120;
    }
  }
  bool x_193 = false;
  bool x_240 = false;
  bool x_193_phi = false;
  int x_196_phi = 0;
  bool x_242_phi = false;
  0u;
  do {
    x_193_phi = false;
    x_196_phi = 0;
    [loop] while (true) {
      int x_221 = 0;
      int x_213 = 0;
      bool x_194 = false;
      int x_197 = 0;
      bool x_194_phi = false;
      int x_197_phi = 0;
      x_193 = x_193_phi;
      const int x_196 = x_196_phi;
      x_242_phi = x_193;
      if ((x_196 <= 3)) {
      } else {
        break;
      }
      const int x_202 = tree[x_196].data;
      if ((15 <= x_202)) {
        bool x_240_phi = false;
        const int x_215_save = x_196;
        const int x_216 = tree[x_215_save].leftIndex;
        if ((x_216 == -1)) {
          const float x_223 = asfloat(x_8[0].y);
          const float x_225 = asfloat(x_8[0].x);
          if ((x_223 < x_225)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_215_save].leftIndex = 3;
          const BST tint_symbol_9 = {15, -1, -1};
          tree[3] = tint_symbol_9;
          [loop] while (true) {
            x_240_phi = x_193;
            if ((0 < int(x_223))) {
            } else {
              break;
            }
            x_240_phi = true;
            break;
          }
          x_240 = x_240_phi;
          x_242_phi = x_240;
          if (x_240) {
            break;
          }
        } else {
          x_221 = tree[x_215_save].leftIndex;
          x_194_phi = x_193;
          x_197_phi = x_221;
          {
            x_194 = x_194_phi;
            x_197 = x_197_phi;
            x_193_phi = x_194;
            x_196_phi = x_197;
          }
          continue;
        }
      } else {
        const int x_207_save = x_196;
        const int x_208 = tree[x_207_save].rightIndex;
        if ((x_208 == -1)) {
          tree[x_207_save].rightIndex = 3;
          const BST tint_symbol_10 = {15, -1, -1};
          tree[3] = tint_symbol_10;
          x_242_phi = true;
          break;
        } else {
          x_213 = tree[x_207_save].rightIndex;
          x_194_phi = x_193;
          x_197_phi = x_213;
          {
            x_194 = x_194_phi;
            x_197 = x_197_phi;
            x_193_phi = x_194;
            x_196_phi = x_197;
          }
          continue;
        }
        return;
      }
      x_194_phi = x_240;
      x_197_phi = x_196;
      {
        x_194 = x_194_phi;
        x_197 = x_197_phi;
        x_193_phi = x_194;
        x_196_phi = x_197;
      }
    }
    if (x_242_phi) {
      break;
    }
  } while (false);
  bool x_247 = false;
  bool x_294 = false;
  bool x_247_phi = false;
  int x_250_phi = 0;
  bool x_296_phi = false;
  0u;
  do {
    x_247_phi = false;
    x_250_phi = 0;
    [loop] while (true) {
      int x_275 = 0;
      int x_267 = 0;
      bool x_248 = false;
      int x_251 = 0;
      bool x_248_phi = false;
      int x_251_phi = 0;
      x_247 = x_247_phi;
      const int x_250 = x_250_phi;
      x_296_phi = x_247;
      if ((x_250 <= 4)) {
      } else {
        break;
      }
      const int x_256 = tree[x_250].data;
      if ((7 <= x_256)) {
        bool x_294_phi = false;
        const int x_269_save = x_250;
        const int x_270 = tree[x_269_save].leftIndex;
        if ((x_270 == -1)) {
          const float x_277 = asfloat(x_8[0].y);
          const float x_279 = asfloat(x_8[0].x);
          if ((x_277 < x_279)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_269_save].leftIndex = 4;
          const BST tint_symbol_11 = {7, -1, -1};
          tree[4] = tint_symbol_11;
          [loop] while (true) {
            x_294_phi = x_247;
            if ((0 < int(x_277))) {
            } else {
              break;
            }
            x_294_phi = true;
            break;
          }
          x_294 = x_294_phi;
          x_296_phi = x_294;
          if (x_294) {
            break;
          }
        } else {
          x_275 = tree[x_269_save].leftIndex;
          x_248_phi = x_247;
          x_251_phi = x_275;
          {
            x_248 = x_248_phi;
            x_251 = x_251_phi;
            x_247_phi = x_248;
            x_250_phi = x_251;
          }
          continue;
        }
      } else {
        const int x_261_save = x_250;
        const int x_262 = tree[x_261_save].rightIndex;
        if ((x_262 == -1)) {
          tree[x_261_save].rightIndex = 4;
          const BST tint_symbol_12 = {7, -1, -1};
          tree[4] = tint_symbol_12;
          x_296_phi = true;
          break;
        } else {
          x_267 = tree[x_261_save].rightIndex;
          x_248_phi = x_247;
          x_251_phi = x_267;
          {
            x_248 = x_248_phi;
            x_251 = x_251_phi;
            x_247_phi = x_248;
            x_250_phi = x_251;
          }
          continue;
        }
        return;
      }
      x_248_phi = x_294;
      x_251_phi = x_250;
      {
        x_248 = x_248_phi;
        x_251 = x_251_phi;
        x_247_phi = x_248;
        x_250_phi = x_251;
      }
    }
    if (x_296_phi) {
      break;
    }
  } while (false);
  bool x_301 = false;
  bool x_348 = false;
  bool x_301_phi = false;
  int x_304_phi = 0;
  bool x_350_phi = false;
  0u;
  do {
    x_301_phi = false;
    x_304_phi = 0;
    [loop] while (true) {
      int x_329 = 0;
      int x_321 = 0;
      bool x_302 = false;
      int x_305 = 0;
      bool x_302_phi = false;
      int x_305_phi = 0;
      x_301 = x_301_phi;
      const int x_304 = x_304_phi;
      x_350_phi = x_301;
      if ((x_304 <= 5)) {
      } else {
        break;
      }
      const int x_310 = tree[x_304].data;
      if ((8 <= x_310)) {
        bool x_348_phi = false;
        const int x_323_save = x_304;
        const int x_324 = tree[x_323_save].leftIndex;
        if ((x_324 == -1)) {
          const float x_331 = asfloat(x_8[0].y);
          const float x_333 = asfloat(x_8[0].x);
          if ((x_331 < x_333)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_323_save].leftIndex = 5;
          const BST tint_symbol_13 = {8, -1, -1};
          tree[5] = tint_symbol_13;
          [loop] while (true) {
            x_348_phi = x_301;
            if ((0 < int(x_331))) {
            } else {
              break;
            }
            x_348_phi = true;
            break;
          }
          x_348 = x_348_phi;
          x_350_phi = x_348;
          if (x_348) {
            break;
          }
        } else {
          x_329 = tree[x_323_save].leftIndex;
          x_302_phi = x_301;
          x_305_phi = x_329;
          {
            x_302 = x_302_phi;
            x_305 = x_305_phi;
            x_301_phi = x_302;
            x_304_phi = x_305;
          }
          continue;
        }
      } else {
        const int x_315_save = x_304;
        const int x_316 = tree[x_315_save].rightIndex;
        if ((x_316 == -1)) {
          tree[x_315_save].rightIndex = 5;
          const BST tint_symbol_14 = {8, -1, -1};
          tree[5] = tint_symbol_14;
          x_350_phi = true;
          break;
        } else {
          x_321 = tree[x_315_save].rightIndex;
          x_302_phi = x_301;
          x_305_phi = x_321;
          {
            x_302 = x_302_phi;
            x_305 = x_305_phi;
            x_301_phi = x_302;
            x_304_phi = x_305;
          }
          continue;
        }
        return;
      }
      x_302_phi = x_348;
      x_305_phi = x_304;
      {
        x_302 = x_302_phi;
        x_305 = x_305_phi;
        x_301_phi = x_302;
        x_304_phi = x_305;
      }
    }
    if (x_350_phi) {
      break;
    }
  } while (false);
  bool x_355 = false;
  bool x_402 = false;
  bool x_355_phi = false;
  int x_358_phi = 0;
  bool x_404_phi = false;
  0u;
  do {
    x_355_phi = false;
    x_358_phi = 0;
    [loop] while (true) {
      int x_383 = 0;
      int x_375 = 0;
      bool x_356 = false;
      int x_359 = 0;
      bool x_356_phi = false;
      int x_359_phi = 0;
      x_355 = x_355_phi;
      const int x_358 = x_358_phi;
      x_404_phi = x_355;
      if ((x_358 <= 6)) {
      } else {
        break;
      }
      const int x_364 = tree[x_358].data;
      if ((2 <= x_364)) {
        bool x_402_phi = false;
        const int x_377_save = x_358;
        const int x_378 = tree[x_377_save].leftIndex;
        if ((x_378 == -1)) {
          const float x_385 = asfloat(x_8[0].y);
          const float x_387 = asfloat(x_8[0].x);
          if ((x_385 < x_387)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_377_save].leftIndex = 6;
          const BST tint_symbol_15 = {2, -1, -1};
          tree[6] = tint_symbol_15;
          [loop] while (true) {
            x_402_phi = x_355;
            if ((0 < int(x_385))) {
            } else {
              break;
            }
            x_402_phi = true;
            break;
          }
          x_402 = x_402_phi;
          x_404_phi = x_402;
          if (x_402) {
            break;
          }
        } else {
          x_383 = tree[x_377_save].leftIndex;
          x_356_phi = x_355;
          x_359_phi = x_383;
          {
            x_356 = x_356_phi;
            x_359 = x_359_phi;
            x_355_phi = x_356;
            x_358_phi = x_359;
          }
          continue;
        }
      } else {
        const int x_369_save = x_358;
        const int x_370 = tree[x_369_save].rightIndex;
        if ((x_370 == -1)) {
          tree[x_369_save].rightIndex = 6;
          const BST tint_symbol_16 = {2, -1, -1};
          tree[6] = tint_symbol_16;
          x_404_phi = true;
          break;
        } else {
          x_375 = tree[x_369_save].rightIndex;
          x_356_phi = x_355;
          x_359_phi = x_375;
          {
            x_356 = x_356_phi;
            x_359 = x_359_phi;
            x_355_phi = x_356;
            x_358_phi = x_359;
          }
          continue;
        }
        return;
      }
      x_356_phi = x_402;
      x_359_phi = x_358;
      {
        x_356 = x_356_phi;
        x_359 = x_359_phi;
        x_355_phi = x_356;
        x_358_phi = x_359;
      }
    }
    if (x_404_phi) {
      break;
    }
  } while (false);
  bool x_409 = false;
  bool x_456 = false;
  bool x_409_phi = false;
  int x_412_phi = 0;
  bool x_458_phi = false;
  0u;
  do {
    x_409_phi = false;
    x_412_phi = 0;
    [loop] while (true) {
      int x_437 = 0;
      int x_429 = 0;
      bool x_410 = false;
      int x_413 = 0;
      bool x_410_phi = false;
      int x_413_phi = 0;
      x_409 = x_409_phi;
      const int x_412 = x_412_phi;
      x_458_phi = x_409;
      if ((x_412 <= 7)) {
      } else {
        break;
      }
      const int x_418 = tree[x_412].data;
      if ((6 <= x_418)) {
        bool x_456_phi = false;
        const int x_431_save = x_412;
        const int x_432 = tree[x_431_save].leftIndex;
        if ((x_432 == -1)) {
          const float x_439 = asfloat(x_8[0].y);
          const float x_441 = asfloat(x_8[0].x);
          if ((x_439 < x_441)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_431_save].leftIndex = 7;
          const BST tint_symbol_17 = {6, -1, -1};
          tree[7] = tint_symbol_17;
          [loop] while (true) {
            x_456_phi = x_409;
            if ((0 < int(x_439))) {
            } else {
              break;
            }
            x_456_phi = true;
            break;
          }
          x_456 = x_456_phi;
          x_458_phi = x_456;
          if (x_456) {
            break;
          }
        } else {
          x_437 = tree[x_431_save].leftIndex;
          x_410_phi = x_409;
          x_413_phi = x_437;
          {
            x_410 = x_410_phi;
            x_413 = x_413_phi;
            x_409_phi = x_410;
            x_412_phi = x_413;
          }
          continue;
        }
      } else {
        const int x_423_save = x_412;
        const int x_424 = tree[x_423_save].rightIndex;
        if ((x_424 == -1)) {
          tree[x_423_save].rightIndex = 7;
          const BST tint_symbol_18 = {6, -1, -1};
          tree[7] = tint_symbol_18;
          x_458_phi = true;
          break;
        } else {
          x_429 = tree[x_423_save].rightIndex;
          x_410_phi = x_409;
          x_413_phi = x_429;
          {
            x_410 = x_410_phi;
            x_413 = x_413_phi;
            x_409_phi = x_410;
            x_412_phi = x_413;
          }
          continue;
        }
        return;
      }
      x_410_phi = x_456;
      x_413_phi = x_412;
      {
        x_410 = x_410_phi;
        x_413 = x_413_phi;
        x_409_phi = x_410;
        x_412_phi = x_413;
      }
    }
    if (x_458_phi) {
      break;
    }
  } while (false);
  bool x_463 = false;
  bool x_510 = false;
  bool x_463_phi = false;
  int x_466_phi = 0;
  bool x_512_phi = false;
  0u;
  do {
    x_463_phi = false;
    x_466_phi = 0;
    [loop] while (true) {
      int x_491 = 0;
      int x_483 = 0;
      bool x_464 = false;
      int x_467 = 0;
      bool x_464_phi = false;
      int x_467_phi = 0;
      x_463 = x_463_phi;
      const int x_466 = x_466_phi;
      x_512_phi = x_463;
      if ((x_466 <= 8)) {
      } else {
        break;
      }
      const int x_472 = tree[x_466].data;
      if ((17 <= x_472)) {
        bool x_510_phi = false;
        const int x_485_save = x_466;
        const int x_486 = tree[x_485_save].leftIndex;
        if ((x_486 == -1)) {
          const float x_493 = asfloat(x_8[0].y);
          const float x_495 = asfloat(x_8[0].x);
          if ((x_493 < x_495)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_485_save].leftIndex = 8;
          const BST tint_symbol_19 = {17, -1, -1};
          tree[8] = tint_symbol_19;
          [loop] while (true) {
            x_510_phi = x_463;
            if ((0 < int(x_493))) {
            } else {
              break;
            }
            x_510_phi = true;
            break;
          }
          x_510 = x_510_phi;
          x_512_phi = x_510;
          if (x_510) {
            break;
          }
        } else {
          x_491 = tree[x_485_save].leftIndex;
          x_464_phi = x_463;
          x_467_phi = x_491;
          {
            x_464 = x_464_phi;
            x_467 = x_467_phi;
            x_463_phi = x_464;
            x_466_phi = x_467;
          }
          continue;
        }
      } else {
        const int x_477_save = x_466;
        const int x_478 = tree[x_477_save].rightIndex;
        if ((x_478 == -1)) {
          tree[x_477_save].rightIndex = 8;
          const BST tint_symbol_20 = {17, -1, -1};
          tree[8] = tint_symbol_20;
          x_512_phi = true;
          break;
        } else {
          x_483 = tree[x_477_save].rightIndex;
          x_464_phi = x_463;
          x_467_phi = x_483;
          {
            x_464 = x_464_phi;
            x_467 = x_467_phi;
            x_463_phi = x_464;
            x_466_phi = x_467;
          }
          continue;
        }
        return;
      }
      x_464_phi = x_510;
      x_467_phi = x_466;
      {
        x_464 = x_464_phi;
        x_467 = x_467_phi;
        x_463_phi = x_464;
        x_466_phi = x_467;
      }
    }
    if (x_512_phi) {
      break;
    }
  } while (false);
  bool x_517 = false;
  bool x_564 = false;
  bool x_517_phi = false;
  int x_520_phi = 0;
  bool x_566_phi = false;
  0u;
  do {
    x_517_phi = false;
    x_520_phi = 0;
    [loop] while (true) {
      int x_545 = 0;
      int x_537 = 0;
      bool x_518 = false;
      int x_521 = 0;
      bool x_518_phi = false;
      int x_521_phi = 0;
      x_517 = x_517_phi;
      const int x_520 = x_520_phi;
      x_566_phi = x_517;
      if ((x_520 <= 9)) {
      } else {
        break;
      }
      const int x_526 = tree[x_520].data;
      if ((13 <= x_526)) {
        bool x_564_phi = false;
        const int x_539_save = x_520;
        const int x_540 = tree[x_539_save].leftIndex;
        if ((x_540 == -1)) {
          const float x_547 = asfloat(x_8[0].y);
          const float x_549 = asfloat(x_8[0].x);
          if ((x_547 < x_549)) {
            [loop] while (true) {
              discard;
            }
            return;
          }
          tree[x_539_save].leftIndex = 9;
          const BST tint_symbol_21 = {13, -1, -1};
          tree[9] = tint_symbol_21;
          [loop] while (true) {
            x_564_phi = x_517;
            if ((0 < int(x_547))) {
            } else {
              break;
            }
            x_564_phi = true;
            break;
          }
          x_564 = x_564_phi;
          x_566_phi = x_564;
          if (x_564) {
            break;
          }
        } else {
          x_545 = tree[x_539_save].leftIndex;
          x_518_phi = x_517;
          x_521_phi = x_545;
          {
            x_518 = x_518_phi;
            x_521 = x_521_phi;
            x_517_phi = x_518;
            x_520_phi = x_521;
          }
          continue;
        }
      } else {
        const int x_531_save = x_520;
        const int x_532 = tree[x_531_save].rightIndex;
        if ((x_532 == -1)) {
          tree[x_531_save].rightIndex = 9;
          const BST tint_symbol_22 = {13, -1, -1};
          tree[9] = tint_symbol_22;
          x_566_phi = true;
          break;
        } else {
          x_537 = tree[x_531_save].rightIndex;
          x_518_phi = x_517;
          x_521_phi = x_537;
          {
            x_518 = x_518_phi;
            x_521 = x_521_phi;
            x_517_phi = x_518;
            x_520_phi = x_521;
          }
          continue;
        }
        return;
      }
      x_518_phi = x_564;
      x_521_phi = x_520;
      {
        x_518 = x_518_phi;
        x_521 = x_521_phi;
        x_517_phi = x_518;
        x_520_phi = x_521;
      }
    }
    if (x_566_phi) {
      break;
    }
  } while (false);
  x_569_phi = 0;
  x_572_phi = 0;
  x_574_phi = 0;
  [loop] while (true) {
    int x_597 = 0;
    int x_607 = 0;
    int x_612 = 0;
    int x_575 = 0;
    int x_570_phi = 0;
    int x_573_phi = 0;
    const int x_569 = x_569_phi;
    x_572 = x_572_phi;
    const int x_574 = x_574_phi;
    if ((x_574 < 20)) {
    } else {
      break;
    }
    int x_582_phi = 0;
    int x_597_phi = 0;
    bool x_598_phi = false;
    0u;
    do {
      x_582_phi = 0;
      [loop] while (true) {
        const int x_582 = x_582_phi;
        x_597_phi = x_569;
        x_598_phi = false;
        if ((x_582 != -1)) {
        } else {
          break;
        }
        const BST x_589 = tree[x_582];
        const int x_590 = x_589.data;
        const int x_591 = x_589.leftIndex;
        const int x_592 = x_589.rightIndex;
        if ((x_590 == x_574)) {
          x_597_phi = x_574;
          x_598_phi = true;
          break;
        }
        {
          x_582_phi = ((x_574 > x_590) ? x_592 : x_591);
        }
      }
      x_597 = x_597_phi;
      const bool x_598 = x_598_phi;
      x_570_phi = x_597;
      if (x_598) {
        break;
      }
      x_570_phi = -1;
    } while (false);
    int x_570 = 0;
    int x_606 = 0;
    int x_611 = 0;
    int x_607_phi = 0;
    int x_612_phi = 0;
    x_570 = x_570_phi;
    switch(x_574) {
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
        x_607_phi = x_572;
        if ((x_570 == asint(x_574))) {
          x_606 = asint((x_572 + asint(1)));
          x_607_phi = x_606;
        }
        x_607 = x_607_phi;
        x_573_phi = x_607;
        break;
      }
      default: {
        x_612_phi = x_572;
        if ((x_570 == asint(-1))) {
          x_611 = asint((x_572 + asint(1)));
          x_612_phi = x_611;
        }
        x_612 = x_612_phi;
        x_573_phi = x_612;
        break;
      }
    }
    const int x_573 = x_573_phi;
    {
      x_575 = (x_574 + 1);
      x_569_phi = x_570;
      x_572_phi = x_573;
      x_574_phi = x_575;
    }
  }
  if ((x_572 == asint(20))) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 1.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_23 = {x_GLF_color};
  return tint_symbol_23;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000016CFDF12D00(123,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000016CFDF12D00(54,27-30): error X3696: infinite loop detected - loop never exits

