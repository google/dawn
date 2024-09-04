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
int performPartition_i1_i1_(inout int l, inout int h) {
  int x_314 = 0;
  int x_315 = 0;
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_316 = h;
  int x_318 = obj.numbers[x_316];
  pivot = x_318;
  int x_319 = l;
  i_1 = (x_319 - 1);
  int x_321 = l;
  j_1 = x_321;
  {
    while(true) {
      int x_326 = j_1;
      int x_327 = h;
      if ((x_326 <= (x_327 - 1))) {
      } else {
        break;
      }
      int x_331 = j_1;
      int x_333 = obj.numbers[x_331];
      int x_334 = pivot;
      if ((x_333 <= x_334)) {
        int x_338 = i_1;
        i_1 = (x_338 + 1);
        int x_340 = i_1;
        param = x_340;
        int x_341 = j_1;
        param_1 = x_341;
        int x_342 = param;
        int x_344 = obj.numbers[x_342];
        x_315 = x_344;
        int x_345 = param;
        int x_346 = param_1;
        int x_348 = obj.numbers[x_346];
        obj.numbers[x_345] = x_348;
        int x_350 = param_1;
        int x_351 = x_315;
        obj.numbers[x_350] = x_351;
      }
      {
        int x_353 = j_1;
        j_1 = (x_353 + 1);
      }
      continue;
    }
  }
  int x_355 = i_1;
  param_2 = (x_355 + 1);
  int x_357 = h;
  param_3 = x_357;
  int x_358 = param_2;
  int x_360 = obj.numbers[x_358];
  x_314 = x_360;
  int x_361 = param_2;
  int x_362 = param_3;
  int x_364 = obj.numbers[x_362];
  obj.numbers[x_361] = x_364;
  int x_366 = param_3;
  int x_367 = x_314;
  obj.numbers[x_366] = x_367;
  if (false) {
  } else {
    int x_372 = i_1;
    return (x_372 + 1);
  }
  return 0;
}
void main_1() {
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int x_95 = 0;
  int x_96 = 0;
  int x_97 = 0;
  int i_2 = 0;
  vec2 uv = vec2(0.0f);
  vec3 color = vec3(0.0f);
  vec4 x_98 = x_GLF_pos;
  x_GLF_FragCoord = ((x_98 + vec4(1.0f, 1.0f, 0.0f, 0.0f)) * vec4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      int x_105 = i_2;
      if ((x_105 < 10)) {
      } else {
        break;
      }
      int x_108 = i_2;
      int x_109 = i_2;
      obj.numbers[x_108] = (10 - x_109);
      int x_112 = i_2;
      int x_113 = i_2;
      int x_115 = obj.numbers[x_113];
      int x_116 = i_2;
      int x_118 = obj.numbers[x_116];
      obj.numbers[x_112] = (x_115 * x_118);
      {
        int x_121 = i_2;
        i_2 = (x_121 + 1);
      }
      continue;
    }
  }
  x_91 = 0;
  x_92 = 9;
  x_93 = -1;
  int x_123 = x_93;
  int x_124 = (x_123 + 1);
  x_93 = x_124;
  int x_125 = x_91;
  x_94[x_124] = x_125;
  int x_127 = x_93;
  int x_128 = (x_127 + 1);
  x_93 = x_128;
  int x_129 = x_92;
  x_94[x_128] = x_129;
  {
    while(true) {
      int x_135 = x_93;
      if ((x_135 >= 0)) {
      } else {
        break;
      }
      int x_138 = x_93;
      x_93 = (x_138 - 1);
      int x_141 = x_94[x_138];
      x_92 = x_141;
      int x_142 = x_93;
      x_93 = (x_142 - 1);
      int x_145 = x_94[x_142];
      x_91 = x_145;
      int x_146 = x_91;
      x_96 = x_146;
      int x_147 = x_92;
      x_97 = x_147;
      int x_148 = performPartition_i1_i1_(x_96, x_97);
      x_95 = x_148;
      int x_149 = x_95;
      int x_151 = x_91;
      if (((x_149 - 1) > x_151)) {
        int x_155 = x_93;
        int x_156 = (x_155 + 1);
        x_93 = x_156;
        int x_157 = x_91;
        x_94[x_156] = x_157;
        int x_159 = x_93;
        int x_160 = (x_159 + 1);
        x_93 = x_160;
        int x_161 = x_95;
        x_94[x_160] = (x_161 - 1);
      }
      int x_164 = x_95;
      int x_166 = x_92;
      if (((x_164 + 1) < x_166)) {
        int x_170 = x_93;
        int x_171 = (x_170 + 1);
        x_93 = x_171;
        int x_172 = x_95;
        x_94[x_171] = (x_172 + 1);
        int x_175 = x_93;
        int x_176 = (x_175 + 1);
        x_93 = x_176;
        int x_177 = x_92;
        x_94[x_176] = x_177;
      }
      {
      }
      continue;
    }
  }
  vec4 x_179 = x_GLF_FragCoord;
  vec2 x_182 = x_34.resolution;
  uv = (vec2(x_179[0u], x_179[1u]) / x_182);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_185 = obj.numbers[0];
  float x_188 = color.x;
  color[0u] = (x_188 + float(x_185));
  float x_192 = uv.x;
  if ((x_192 > 0.25f)) {
    int x_197 = obj.numbers[1];
    float x_200 = color.x;
    color[0u] = (x_200 + float(x_197));
  }
  float x_204 = uv.x;
  if ((x_204 > 0.5f)) {
    int x_209 = obj.numbers[2];
    float x_212 = color.y;
    color[1u] = (x_212 + float(x_209));
  }
  float x_216 = uv.x;
  if ((x_216 > 0.75f)) {
    int x_221 = obj.numbers[3];
    float x_224 = color.z;
    color[2u] = (x_224 + float(x_221));
  }
  int x_228 = obj.numbers[4];
  float x_231 = color.y;
  color[1u] = (x_231 + float(x_228));
  float x_235 = uv.y;
  if ((x_235 > 0.25f)) {
    int x_240 = obj.numbers[5];
    float x_243 = color.x;
    color[0u] = (x_243 + float(x_240));
  }
  float x_247 = uv.y;
  if ((x_247 > 0.5f)) {
    int x_252 = obj.numbers[6];
    float x_255 = color.y;
    color[1u] = (x_255 + float(x_252));
  }
  float x_259 = uv.y;
  if ((x_259 > 0.75f)) {
    int x_264 = obj.numbers[7];
    float x_267 = color.z;
    color[2u] = (x_267 + float(x_264));
  }
  int x_271 = obj.numbers[8];
  float x_274 = color.z;
  color[2u] = (x_274 + float(x_271));
  float x_278 = uv.x;
  float x_280 = uv.y;
  if ((abs((x_278 - x_280)) < 0.25f)) {
    int x_287 = obj.numbers[9];
    float x_290 = color.x;
    color[0u] = (x_290 + float(x_287));
  }
  vec3 x_293 = color;
  vec3 x_294 = normalize(x_293);
  frag_color = vec4(x_294[0u], x_294[1u], x_294[2u], 1.0f);
  vec4 x_299 = x_GLF_pos;
  tint_symbol = x_299;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  int x_302 = i;
  int x_304 = obj.numbers[x_302];
  temp = x_304;
  int x_305 = i;
  int x_306 = j;
  int x_308 = obj.numbers[x_306];
  obj.numbers[x_305] = x_308;
  int x_310 = j;
  int x_311 = temp;
  obj.numbers[x_310] = x_311;
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
  int x_376 = top;
  int x_377 = (x_376 + 1);
  top = x_377;
  int x_378 = l_1;
  stack[x_377] = x_378;
  int x_380 = top;
  int x_381 = (x_380 + 1);
  top = x_381;
  int x_382 = h_1;
  stack[x_381] = x_382;
  {
    while(true) {
      int x_388 = top;
      if ((x_388 >= 0)) {
      } else {
        break;
      }
      int x_391 = top;
      top = (x_391 - 1);
      int x_394 = stack[x_391];
      h_1 = x_394;
      int x_395 = top;
      top = (x_395 - 1);
      int x_398 = stack[x_395];
      l_1 = x_398;
      int x_399 = l_1;
      param_4 = x_399;
      int x_400 = h_1;
      param_5 = x_400;
      int x_401 = performPartition_i1_i1_(param_4, param_5);
      p = x_401;
      int x_402 = p;
      int x_404 = l_1;
      if (((x_402 - 1) > x_404)) {
        int x_408 = top;
        int x_409 = (x_408 + 1);
        top = x_409;
        int x_410 = l_1;
        stack[x_409] = x_410;
        int x_412 = top;
        int x_413 = (x_412 + 1);
        top = x_413;
        int x_414 = p;
        stack[x_413] = (x_414 - 1);
      }
      int x_417 = p;
      int x_419 = h_1;
      if (((x_417 + 1) < x_419)) {
        int x_423 = top;
        int x_424 = (x_423 + 1);
        top = x_424;
        int x_425 = p;
        stack[x_424] = (x_425 + 1);
        int x_428 = top;
        int x_429 = (x_428 + 1);
        top = x_429;
        int x_430 = h_1;
        stack[x_429] = x_430;
      }
      {
      }
      continue;
    }
  }
}
error: Error parsing GLSL shader:
ERROR: 0:262: 'main' : function cannot take any parameter(s) 
ERROR: 0:262: 'structure' :  entry point cannot return a value
ERROR: 0:262: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
