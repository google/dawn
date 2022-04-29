SKIP: FAILED https://crbug.com/tint/1522

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

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  BST tree[10] = (BST[10])0;
  int x_356 = 0;
  int x_58_phi = 0;
  bool x_86_phi = false;
  int x_353_phi = 0;
  int x_356_phi = 0;
  int x_358_phi = 0;
  const BST tint_symbol_1 = {9, -1, -1};
  tree[0] = tint_symbol_1;
  0u;
  do {
    x_58_phi = 0;
    [loop] while (true) {
      int x_84 = 0;
      int x_76 = 0;
      int x_59 = 0;
      int x_59_phi = 0;
      const int x_58 = x_58_phi;
      x_86_phi = false;
      if ((x_58 <= 1)) {
      } else {
        break;
      }
      const int x_65 = tree[x_58].data;
      if ((5 <= x_65)) {
        const int x_78_save = x_58;
        const int x_79 = tree[x_78_save].leftIndex;
        if ((x_79 == -1)) {
          tree[x_78_save].leftIndex = 1;
          const BST tint_symbol_2 = {5, -1, -1};
          tree[1] = tint_symbol_2;
          x_86_phi = true;
          break;
        } else {
          x_84 = tree[x_78_save].leftIndex;
          x_59_phi = x_84;
          {
            x_59 = x_59_phi;
            x_58_phi = x_59;
          }
          continue;
        }
        return;
      } else {
        const int x_70_save = x_58;
        const int x_71 = tree[x_70_save].rightIndex;
        if ((x_71 == -1)) {
          tree[x_70_save].rightIndex = 1;
          const BST tint_symbol_3 = {5, -1, -1};
          tree[1] = tint_symbol_3;
          x_86_phi = true;
          break;
        } else {
          x_76 = tree[x_70_save].rightIndex;
          x_59_phi = x_76;
          {
            x_59 = x_59_phi;
            x_58_phi = x_59;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_59 = x_59_phi;
        x_58_phi = x_59;
      }
    }
    if (x_86_phi) {
      break;
    }
  } while (false);
  int x_91_phi = 0;
  bool x_119_phi = false;
  0u;
  do {
    x_91_phi = 0;
    [loop] while (true) {
      int x_117 = 0;
      int x_109 = 0;
      int x_92 = 0;
      int x_92_phi = 0;
      const int x_91 = x_91_phi;
      x_119_phi = false;
      if ((x_91 <= 2)) {
      } else {
        break;
      }
      const int x_98 = tree[x_91].data;
      if ((12 <= x_98)) {
        const int x_111_save = x_91;
        const int x_112 = tree[x_111_save].leftIndex;
        if ((x_112 == -1)) {
          tree[x_111_save].leftIndex = 2;
          const BST tint_symbol_4 = {12, -1, -1};
          tree[2] = tint_symbol_4;
          x_119_phi = true;
          break;
        } else {
          x_117 = tree[x_111_save].leftIndex;
          x_92_phi = x_117;
          {
            x_92 = x_92_phi;
            x_91_phi = x_92;
          }
          continue;
        }
        return;
      } else {
        const int x_103_save = x_91;
        const int x_104 = tree[x_103_save].rightIndex;
        if ((x_104 == -1)) {
          tree[x_103_save].rightIndex = 2;
          const BST tint_symbol_5 = {12, -1, -1};
          tree[2] = tint_symbol_5;
          x_119_phi = true;
          break;
        } else {
          x_109 = tree[x_103_save].rightIndex;
          x_92_phi = x_109;
          {
            x_92 = x_92_phi;
            x_91_phi = x_92;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_92 = x_92_phi;
        x_91_phi = x_92;
      }
    }
    if (x_119_phi) {
      break;
    }
  } while (false);
  int x_124_phi = 0;
  bool x_152_phi = false;
  0u;
  do {
    x_124_phi = 0;
    [loop] while (true) {
      int x_150 = 0;
      int x_142 = 0;
      int x_125 = 0;
      int x_125_phi = 0;
      const int x_124 = x_124_phi;
      x_152_phi = false;
      if ((x_124 <= 3)) {
      } else {
        break;
      }
      const int x_131 = tree[x_124].data;
      if ((15 <= x_131)) {
        const int x_144_save = x_124;
        const int x_145 = tree[x_144_save].leftIndex;
        if ((x_145 == -1)) {
          tree[x_144_save].leftIndex = 3;
          const BST tint_symbol_6 = {15, -1, -1};
          tree[3] = tint_symbol_6;
          x_152_phi = true;
          break;
        } else {
          x_150 = tree[x_144_save].leftIndex;
          x_125_phi = x_150;
          {
            x_125 = x_125_phi;
            x_124_phi = x_125;
          }
          continue;
        }
        return;
      } else {
        const int x_136_save = x_124;
        const int x_137 = tree[x_136_save].rightIndex;
        if ((x_137 == -1)) {
          tree[x_136_save].rightIndex = 3;
          const BST tint_symbol_7 = {15, -1, -1};
          tree[3] = tint_symbol_7;
          x_152_phi = true;
          break;
        } else {
          x_142 = tree[x_136_save].rightIndex;
          x_125_phi = x_142;
          {
            x_125 = x_125_phi;
            x_124_phi = x_125;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_125 = x_125_phi;
        x_124_phi = x_125;
      }
    }
    if (x_152_phi) {
      break;
    }
  } while (false);
  int x_157_phi = 0;
  bool x_185_phi = false;
  0u;
  do {
    x_157_phi = 0;
    [loop] while (true) {
      int x_183 = 0;
      int x_175 = 0;
      int x_158 = 0;
      int x_158_phi = 0;
      const int x_157 = x_157_phi;
      x_185_phi = false;
      if ((x_157 <= 4)) {
      } else {
        break;
      }
      const int x_164 = tree[x_157].data;
      if ((7 <= x_164)) {
        const int x_177_save = x_157;
        const int x_178 = tree[x_177_save].leftIndex;
        if ((x_178 == -1)) {
          tree[x_177_save].leftIndex = 4;
          const BST tint_symbol_8 = {7, -1, -1};
          tree[4] = tint_symbol_8;
          x_185_phi = true;
          break;
        } else {
          x_183 = tree[x_177_save].leftIndex;
          x_158_phi = x_183;
          {
            x_158 = x_158_phi;
            x_157_phi = x_158;
          }
          continue;
        }
        return;
      } else {
        const int x_169_save = x_157;
        const int x_170 = tree[x_169_save].rightIndex;
        if ((x_170 == -1)) {
          tree[x_169_save].rightIndex = 4;
          const BST tint_symbol_9 = {7, -1, -1};
          tree[4] = tint_symbol_9;
          x_185_phi = true;
          break;
        } else {
          x_175 = tree[x_169_save].rightIndex;
          x_158_phi = x_175;
          {
            x_158 = x_158_phi;
            x_157_phi = x_158;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_158 = x_158_phi;
        x_157_phi = x_158;
      }
    }
    if (x_185_phi) {
      break;
    }
  } while (false);
  int x_190_phi = 0;
  bool x_218_phi = false;
  0u;
  do {
    x_190_phi = 0;
    [loop] while (true) {
      int x_216 = 0;
      int x_208 = 0;
      int x_191 = 0;
      int x_191_phi = 0;
      const int x_190 = x_190_phi;
      x_218_phi = false;
      if ((x_190 <= 5)) {
      } else {
        break;
      }
      const int x_197 = tree[x_190].data;
      if ((8 <= x_197)) {
        const int x_210_save = x_190;
        const int x_211 = tree[x_210_save].leftIndex;
        if ((x_211 == -1)) {
          tree[x_210_save].leftIndex = 5;
          const BST tint_symbol_10 = {8, -1, -1};
          tree[5] = tint_symbol_10;
          x_218_phi = true;
          break;
        } else {
          x_216 = tree[x_210_save].leftIndex;
          x_191_phi = x_216;
          {
            x_191 = x_191_phi;
            x_190_phi = x_191;
          }
          continue;
        }
        return;
      } else {
        const int x_202_save = x_190;
        const int x_203 = tree[x_202_save].rightIndex;
        if ((x_203 == -1)) {
          tree[x_202_save].rightIndex = 5;
          const BST tint_symbol_11 = {8, -1, -1};
          tree[5] = tint_symbol_11;
          x_218_phi = true;
          break;
        } else {
          x_208 = tree[x_202_save].rightIndex;
          x_191_phi = x_208;
          {
            x_191 = x_191_phi;
            x_190_phi = x_191;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_191 = x_191_phi;
        x_190_phi = x_191;
      }
    }
    if (x_218_phi) {
      break;
    }
  } while (false);
  int x_223_phi = 0;
  bool x_251_phi = false;
  0u;
  do {
    x_223_phi = 0;
    [loop] while (true) {
      int x_249 = 0;
      int x_241 = 0;
      int x_224 = 0;
      int x_224_phi = 0;
      const int x_223 = x_223_phi;
      x_251_phi = false;
      if ((x_223 <= 6)) {
      } else {
        break;
      }
      const int x_230 = tree[x_223].data;
      if ((2 <= x_230)) {
        const int x_243_save = x_223;
        const int x_244 = tree[x_243_save].leftIndex;
        if ((x_244 == -1)) {
          tree[x_243_save].leftIndex = 6;
          const BST tint_symbol_12 = {2, -1, -1};
          tree[6] = tint_symbol_12;
          x_251_phi = true;
          break;
        } else {
          x_249 = tree[x_243_save].leftIndex;
          x_224_phi = x_249;
          {
            x_224 = x_224_phi;
            x_223_phi = x_224;
          }
          continue;
        }
        return;
      } else {
        const int x_235_save = x_223;
        const int x_236 = tree[x_235_save].rightIndex;
        if ((x_236 == -1)) {
          tree[x_235_save].rightIndex = 6;
          const BST tint_symbol_13 = {2, -1, -1};
          tree[6] = tint_symbol_13;
          x_251_phi = true;
          break;
        } else {
          x_241 = tree[x_235_save].rightIndex;
          x_224_phi = x_241;
          {
            x_224 = x_224_phi;
            x_223_phi = x_224;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_224 = x_224_phi;
        x_223_phi = x_224;
      }
    }
    if (x_251_phi) {
      break;
    }
  } while (false);
  int x_256_phi = 0;
  bool x_284_phi = false;
  0u;
  do {
    x_256_phi = 0;
    [loop] while (true) {
      int x_282 = 0;
      int x_274 = 0;
      int x_257 = 0;
      int x_257_phi = 0;
      const int x_256 = x_256_phi;
      x_284_phi = false;
      if ((x_256 <= 7)) {
      } else {
        break;
      }
      const int x_263 = tree[x_256].data;
      if ((6 <= x_263)) {
        const int x_276_save = x_256;
        const int x_277 = tree[x_276_save].leftIndex;
        if ((x_277 == -1)) {
          tree[x_276_save].leftIndex = 7;
          const BST tint_symbol_14 = {6, -1, -1};
          tree[7] = tint_symbol_14;
          x_284_phi = true;
          break;
        } else {
          x_282 = tree[x_276_save].leftIndex;
          x_257_phi = x_282;
          {
            x_257 = x_257_phi;
            x_256_phi = x_257;
          }
          continue;
        }
        return;
      } else {
        const int x_268_save = x_256;
        const int x_269 = tree[x_268_save].rightIndex;
        if ((x_269 == -1)) {
          tree[x_268_save].rightIndex = 7;
          const BST tint_symbol_15 = {6, -1, -1};
          tree[7] = tint_symbol_15;
          x_284_phi = true;
          break;
        } else {
          x_274 = tree[x_268_save].rightIndex;
          x_257_phi = x_274;
          {
            x_257 = x_257_phi;
            x_256_phi = x_257;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_257 = x_257_phi;
        x_256_phi = x_257;
      }
    }
    if (x_284_phi) {
      break;
    }
  } while (false);
  int x_289_phi = 0;
  bool x_317_phi = false;
  0u;
  do {
    x_289_phi = 0;
    [loop] while (true) {
      int x_315 = 0;
      int x_307 = 0;
      int x_290 = 0;
      int x_290_phi = 0;
      const int x_289 = x_289_phi;
      x_317_phi = false;
      if ((x_289 <= 8)) {
      } else {
        break;
      }
      const int x_296 = tree[x_289].data;
      if ((17 <= x_296)) {
        const int x_309_save = x_289;
        const int x_310 = tree[x_309_save].leftIndex;
        if ((x_310 == -1)) {
          tree[x_309_save].leftIndex = 8;
          const BST tint_symbol_16 = {17, -1, -1};
          tree[8] = tint_symbol_16;
          x_317_phi = true;
          break;
        } else {
          x_315 = tree[x_309_save].leftIndex;
          x_290_phi = x_315;
          {
            x_290 = x_290_phi;
            x_289_phi = x_290;
          }
          continue;
        }
        return;
      } else {
        const int x_301_save = x_289;
        const int x_302 = tree[x_301_save].rightIndex;
        if ((x_302 == -1)) {
          tree[x_301_save].rightIndex = 8;
          const BST tint_symbol_17 = {17, -1, -1};
          tree[8] = tint_symbol_17;
          x_317_phi = true;
          break;
        } else {
          x_307 = tree[x_301_save].rightIndex;
          x_290_phi = x_307;
          {
            x_290 = x_290_phi;
            x_289_phi = x_290;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_290 = x_290_phi;
        x_289_phi = x_290;
      }
    }
    if (x_317_phi) {
      break;
    }
  } while (false);
  int x_322_phi = 0;
  bool x_350_phi = false;
  0u;
  do {
    x_322_phi = 0;
    [loop] while (true) {
      int x_348 = 0;
      int x_340 = 0;
      int x_323 = 0;
      int x_323_phi = 0;
      const int x_322 = x_322_phi;
      x_350_phi = false;
      if ((x_322 <= 9)) {
      } else {
        break;
      }
      const int x_329 = tree[x_322].data;
      if ((13 <= x_329)) {
        const int x_342_save = x_322;
        const int x_343 = tree[x_342_save].leftIndex;
        if ((x_343 == -1)) {
          tree[x_342_save].leftIndex = 9;
          const BST tint_symbol_18 = {13, -1, -1};
          tree[9] = tint_symbol_18;
          x_350_phi = true;
          break;
        } else {
          x_348 = tree[x_342_save].leftIndex;
          x_323_phi = x_348;
          {
            x_323 = x_323_phi;
            x_322_phi = x_323;
          }
          continue;
        }
        return;
      } else {
        const int x_334_save = x_322;
        const int x_335 = tree[x_334_save].rightIndex;
        if ((x_335 == -1)) {
          tree[x_334_save].rightIndex = 9;
          const BST tint_symbol_19 = {13, -1, -1};
          tree[9] = tint_symbol_19;
          x_350_phi = true;
          break;
        } else {
          x_340 = tree[x_334_save].rightIndex;
          x_323_phi = x_340;
          {
            x_323 = x_323_phi;
            x_322_phi = x_323;
          }
          continue;
        }
        return;
      }
      return;
      {
        x_323 = x_323_phi;
        x_322_phi = x_323;
      }
    }
    if (x_350_phi) {
      break;
    }
  } while (false);
  x_353_phi = 0;
  x_356_phi = 0;
  x_358_phi = 0;
  [loop] while (true) {
    int x_381 = 0;
    int x_391 = 0;
    int x_396 = 0;
    int x_359 = 0;
    int x_354_phi = 0;
    int x_357_phi = 0;
    const int x_353 = x_353_phi;
    x_356 = x_356_phi;
    const int x_358 = x_358_phi;
    if ((x_358 < 20)) {
    } else {
      break;
    }
    int x_366_phi = 0;
    int x_381_phi = 0;
    bool x_382_phi = false;
    0u;
    do {
      x_366_phi = 0;
      [loop] while (true) {
        const int x_366 = x_366_phi;
        x_381_phi = x_353;
        x_382_phi = false;
        if ((x_366 != -1)) {
        } else {
          break;
        }
        const BST x_373 = tree[x_366];
        const int x_374 = x_373.data;
        const int x_375 = x_373.leftIndex;
        const int x_376 = x_373.rightIndex;
        if ((x_374 == x_358)) {
          x_381_phi = x_358;
          x_382_phi = true;
          break;
        }
        {
          x_366_phi = ((x_358 > x_374) ? x_376 : x_375);
        }
      }
      x_381 = x_381_phi;
      const bool x_382 = x_382_phi;
      x_354_phi = x_381;
      if (x_382) {
        break;
      }
      x_354_phi = -1;
    } while (false);
    int x_354 = 0;
    int x_390 = 0;
    int x_395 = 0;
    int x_391_phi = 0;
    int x_396_phi = 0;
    x_354 = x_354_phi;
    switch(x_358) {
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
        x_391_phi = x_356;
        if ((x_354 == asint(x_358))) {
          x_390 = asint((x_356 + asint(1)));
          x_391_phi = x_390;
        }
        x_391 = x_391_phi;
        x_357_phi = x_391;
        break;
      }
      default: {
        x_396_phi = x_356;
        if ((x_354 == asint(-1))) {
          x_395 = asint((x_356 + asint(1)));
          x_396_phi = x_395;
        }
        x_396 = x_396_phi;
        x_357_phi = x_396;
        break;
      }
    }
    const int x_357 = x_357_phi;
    {
      x_359 = (x_358 + 1);
      x_353_phi = x_354;
      x_356_phi = x_357;
      x_358_phi = x_359;
    }
  }
  if ((x_356 == asint(20))) {
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
C:\src\tint\test\Shader@0x0000022EDBF08060(82,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(22,12-23): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x0000022EDBF08060(148,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(214,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(280,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(346,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(412,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(478,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(544,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(610,5-17): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
C:\src\tint\test\Shader@0x0000022EDBF08060(662,7-19): warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
internal error: compilation aborted unexpectedly

