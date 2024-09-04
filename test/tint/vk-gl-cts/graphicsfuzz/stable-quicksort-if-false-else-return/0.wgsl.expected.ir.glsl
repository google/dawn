SKIP: FAILED

#version 310 es

struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 frag_color_1;
  vec4 tint_symbol;
};

QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 x_GLF_FragCoord = vec4(0.0f);
vec4 x_GLF_pos = vec4(0.0f);
uniform buf0 x_34;
vec4 frag_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  int x_90 = 0;
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94 = 0;
  int x_95 = 0;
  int x_96 = 0;
  int x_97 = 0;
  int x_98 = 0;
  int x_99 = 0;
  int x_100 = 0;
  int x_101 = 0;
  int x_102 = 0;
  int x_103[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int x_104 = 0;
  int x_105 = 0;
  int x_106 = 0;
  int i_2 = 0;
  vec2 uv = vec2(0.0f);
  vec3 color = vec3(0.0f);
  vec4 x_107 = x_GLF_pos;
  x_GLF_FragCoord = ((x_107 + vec4(1.0f, 1.0f, 0.0f, 0.0f)) * vec4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      int x_114 = i_2;
      if ((x_114 < 10)) {
      } else {
        break;
      }
      int x_117 = i_2;
      int x_118 = i_2;
      obj.numbers[x_117] = (10 - x_118);
      int x_121 = i_2;
      int x_122 = i_2;
      int x_124 = obj.numbers[x_122];
      int x_125 = i_2;
      int x_127 = obj.numbers[x_125];
      obj.numbers[x_121] = (x_124 * x_127);
      {
        int x_130 = i_2;
        i_2 = (x_130 + 1);
      }
      continue;
    }
  }
  x_100 = 0;
  x_101 = 9;
  x_102 = -1;
  int x_132 = x_102;
  int x_133 = (x_132 + 1);
  x_102 = x_133;
  int x_134 = x_100;
  x_103[x_133] = x_134;
  int x_136 = x_102;
  int x_137 = (x_136 + 1);
  x_102 = x_137;
  int x_138 = x_101;
  x_103[x_137] = x_138;
  {
    while(true) {
      int x_144 = x_102;
      if ((x_144 >= 0)) {
      } else {
        break;
      }
      int x_147 = x_102;
      x_102 = (x_147 - 1);
      int x_150 = x_103[x_147];
      x_101 = x_150;
      int x_151 = x_102;
      x_102 = (x_151 - 1);
      int x_154 = x_103[x_151];
      x_100 = x_154;
      int x_155 = x_100;
      x_105 = x_155;
      int x_156 = x_101;
      x_106 = x_156;
      int x_157 = x_106;
      int x_159 = obj.numbers[x_157];
      x_92 = x_159;
      int x_160 = x_105;
      x_93 = (x_160 - 1);
      int x_162 = x_105;
      x_94 = x_162;
      {
        while(true) {
          int x_167 = x_94;
          int x_168 = x_106;
          if ((x_167 <= (x_168 - 1))) {
          } else {
            break;
          }
          int x_172 = x_94;
          int x_174 = obj.numbers[x_172];
          int x_175 = x_92;
          if ((x_174 <= x_175)) {
            int x_179 = x_93;
            x_93 = (x_179 + 1);
            int x_181 = x_93;
            x_95 = x_181;
            int x_182 = x_94;
            x_96 = x_182;
            int x_183 = x_95;
            int x_185 = obj.numbers[x_183];
            x_91 = x_185;
            int x_186 = x_95;
            int x_187 = x_96;
            int x_189 = obj.numbers[x_187];
            obj.numbers[x_186] = x_189;
            int x_191 = x_96;
            int x_192 = x_91;
            obj.numbers[x_191] = x_192;
          }
          {
            int x_194 = x_94;
            x_94 = (x_194 + 1);
          }
          continue;
        }
      }
      int x_196 = x_93;
      x_97 = (x_196 + 1);
      int x_198 = x_106;
      x_98 = x_198;
      int x_199 = x_97;
      int x_201 = obj.numbers[x_199];
      x_90 = x_201;
      int x_202 = x_97;
      int x_203 = x_98;
      int x_205 = obj.numbers[x_203];
      obj.numbers[x_202] = x_205;
      int x_207 = x_98;
      int x_208 = x_90;
      obj.numbers[x_207] = x_208;
      int x_210 = x_93;
      x_99 = (x_210 + 1);
      int x_212 = x_99;
      x_104 = x_212;
      int x_213 = x_104;
      int x_215 = x_100;
      if (((x_213 - 1) > x_215)) {
        int x_219 = x_102;
        int x_220 = (x_219 + 1);
        x_102 = x_220;
        int x_221 = x_100;
        x_103[x_220] = x_221;
        int x_223 = x_102;
        int x_224 = (x_223 + 1);
        x_102 = x_224;
        int x_225 = x_104;
        x_103[x_224] = (x_225 - 1);
      }
      int x_228 = x_104;
      int x_230 = x_101;
      if (((x_228 + 1) < x_230)) {
        int x_234 = x_102;
        int x_235 = (x_234 + 1);
        x_102 = x_235;
        int x_236 = x_104;
        x_103[x_235] = (x_236 + 1);
        int x_239 = x_102;
        int x_240 = (x_239 + 1);
        x_102 = x_240;
        int x_241 = x_101;
        x_103[x_240] = x_241;
      }
      {
      }
      continue;
    }
  }
  vec4 x_243 = x_GLF_FragCoord;
  vec2 x_246 = x_34.resolution;
  uv = (vec2(x_243[0u], x_243[1u]) / x_246);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_249 = obj.numbers[0];
  float x_252 = color.x;
  color[0u] = (x_252 + float(x_249));
  float x_256 = uv.x;
  if ((x_256 > 0.25f)) {
    int x_261 = obj.numbers[1];
    float x_264 = color.x;
    color[0u] = (x_264 + float(x_261));
  }
  float x_268 = uv.x;
  if ((x_268 > 0.5f)) {
    int x_273 = obj.numbers[2];
    float x_276 = color.y;
    color[1u] = (x_276 + float(x_273));
  }
  float x_280 = uv.x;
  if ((x_280 > 0.75f)) {
    int x_285 = obj.numbers[3];
    float x_288 = color.z;
    color[2u] = (x_288 + float(x_285));
  }
  int x_292 = obj.numbers[4];
  float x_295 = color.y;
  color[1u] = (x_295 + float(x_292));
  float x_299 = uv.y;
  if ((x_299 > 0.25f)) {
    int x_304 = obj.numbers[5];
    float x_307 = color.x;
    color[0u] = (x_307 + float(x_304));
  }
  float x_311 = uv.y;
  if ((x_311 > 0.5f)) {
    int x_316 = obj.numbers[6];
    float x_319 = color.y;
    color[1u] = (x_319 + float(x_316));
  }
  float x_323 = uv.y;
  if ((x_323 > 0.75f)) {
    int x_328 = obj.numbers[7];
    float x_331 = color.z;
    color[2u] = (x_331 + float(x_328));
  }
  int x_335 = obj.numbers[8];
  float x_338 = color.z;
  color[2u] = (x_338 + float(x_335));
  float x_342 = uv.x;
  float x_344 = uv.y;
  if ((abs((x_342 - x_344)) < 0.25f)) {
    int x_351 = obj.numbers[9];
    float x_354 = color.x;
    color[0u] = (x_354 + float(x_351));
  }
  vec3 x_357 = color;
  vec3 x_358 = normalize(x_357);
  frag_color = vec4(x_358[0u], x_358[1u], x_358[2u], 1.0f);
  vec4 x_363 = x_GLF_pos;
  tint_symbol = x_363;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  int x_366 = i;
  int x_368 = obj.numbers[x_366];
  temp = x_368;
  int x_369 = i;
  int x_370 = j;
  int x_372 = obj.numbers[x_370];
  obj.numbers[x_369] = x_372;
  int x_374 = j;
  int x_375 = temp;
  obj.numbers[x_374] = x_375;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_378 = h;
  int x_380 = obj.numbers[x_378];
  pivot = x_380;
  int x_381 = l;
  i_1 = (x_381 - 1);
  int x_383 = l;
  j_1 = x_383;
  {
    while(true) {
      int x_388 = j_1;
      int x_389 = h;
      if ((x_388 <= (x_389 - 1))) {
      } else {
        break;
      }
      int x_393 = j_1;
      int x_395 = obj.numbers[x_393];
      int x_396 = pivot;
      if ((x_395 <= x_396)) {
        int x_400 = i_1;
        i_1 = (x_400 + 1);
        int x_402 = i_1;
        param = x_402;
        int x_403 = j_1;
        param_1 = x_403;
        swap_i1_i1_(param, param_1);
      }
      {
        int x_405 = j_1;
        j_1 = (x_405 + 1);
      }
      continue;
    }
  }
  int x_407 = i_1;
  param_2 = (x_407 + 1);
  int x_409 = h;
  param_3 = x_409;
  swap_i1_i1_(param_2, param_3);
  int x_411 = i_1;
  return (x_411 + 1);
}
void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  int x_414 = top;
  int x_415 = (x_414 + 1);
  top = x_415;
  int x_416 = l_1;
  stack[x_415] = x_416;
  int x_418 = top;
  int x_419 = (x_418 + 1);
  top = x_419;
  int x_420 = h_1;
  stack[x_419] = x_420;
  {
    while(true) {
      int x_426 = top;
      if ((x_426 >= 0)) {
      } else {
        break;
      }
      int x_429 = top;
      top = (x_429 - 1);
      int x_432 = stack[x_429];
      h_1 = x_432;
      int x_433 = top;
      top = (x_433 - 1);
      int x_436 = stack[x_433];
      l_1 = x_436;
      int x_437 = l_1;
      param_4 = x_437;
      int x_438 = h_1;
      param_5 = x_438;
      int x_439 = performPartition_i1_i1_(param_4, param_5);
      p = x_439;
      int x_440 = p;
      int x_442 = l_1;
      if (((x_440 - 1) > x_442)) {
        int x_446 = top;
        int x_447 = (x_446 + 1);
        top = x_447;
        int x_448 = l_1;
        stack[x_447] = x_448;
        int x_450 = top;
        int x_451 = (x_450 + 1);
        top = x_451;
        int x_452 = p;
        stack[x_451] = (x_452 - 1);
      }
      int x_455 = p;
      int x_457 = h_1;
      if (((x_455 + 1) < x_457)) {
        int x_461 = top;
        int x_462 = (x_461 + 1);
        top = x_462;
        int x_463 = p;
        stack[x_462] = (x_463 + 1);
        int x_466 = top;
        int x_467 = (x_466 + 1);
        top = x_467;
        int x_468 = h_1;
        stack[x_467] = x_468;
      }
      {
      }
      continue;
    }
  }
}
error: Error parsing GLSL shader:
ERROR: 0:257: 'main' : function cannot take any parameter(s) 
ERROR: 0:257: 'structure' :  entry point cannot return a value
ERROR: 0:257: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
