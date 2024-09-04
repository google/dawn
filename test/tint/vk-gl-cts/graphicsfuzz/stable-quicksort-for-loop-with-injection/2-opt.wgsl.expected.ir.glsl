SKIP: FAILED

#version 310 es

struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 injectionSwitch;
};

struct buf1 {
  vec2 resolution;
};

struct main_out {
  vec4 frag_color_1;
  vec4 tint_symbol;
};

QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 x_GLF_FragCoord = vec4(0.0f);
vec4 x_GLF_pos = vec4(0.0f);
uniform buf0 x_33;
uniform buf1 x_36;
vec4 frag_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  int x_250 = i;
  int x_252 = obj.numbers[x_250];
  temp = x_252;
  int x_253 = i;
  int x_254 = j;
  int x_256 = obj.numbers[x_254];
  obj.numbers[x_253] = x_256;
  int x_258 = j;
  int x_259 = temp;
  obj.numbers[x_258] = x_259;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_262 = h;
  int x_264 = obj.numbers[x_262];
  pivot = x_264;
  int x_265 = l;
  i_1 = (x_265 - 1);
  int x_267 = l;
  j_1 = x_267;
  {
    while(true) {
      int x_272 = j_1;
      int x_273 = h;
      if ((x_272 <= (x_273 - 1))) {
      } else {
        break;
      }
      int x_277 = j_1;
      int x_279 = obj.numbers[x_277];
      int x_280 = pivot;
      if ((x_279 <= x_280)) {
        int x_284 = i_1;
        i_1 = (x_284 + 1);
        int x_286 = i_1;
        param = x_286;
        int x_287 = j_1;
        param_1 = x_287;
        swap_i1_i1_(param, param_1);
      }
      {
        int x_289 = j_1;
        j_1 = (x_289 + 1);
      }
      continue;
    }
  }
  int x_291 = i_1;
  param_2 = (x_291 + 1);
  int x_293 = h;
  param_3 = x_293;
  swap_i1_i1_(param_2, param_3);
  int x_295 = i_1;
  return (x_295 + 1);
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
  int x_298 = top;
  int x_299 = (x_298 + 1);
  top = x_299;
  int x_300 = l_1;
  stack[x_299] = x_300;
  int x_302 = top;
  int x_303 = (x_302 + 1);
  top = x_303;
  int x_304 = h_1;
  stack[x_303] = x_304;
  {
    while(true) {
      int x_310 = top;
      if ((x_310 >= 0)) {
      } else {
        break;
      }
      int x_313 = top;
      top = (x_313 - 1);
      int x_316 = stack[x_313];
      h_1 = x_316;
      int x_317 = top;
      top = (x_317 - 1);
      int x_320 = stack[x_317];
      l_1 = x_320;
      int x_321 = l_1;
      param_4 = x_321;
      int x_322 = h_1;
      param_5 = x_322;
      int x_323 = performPartition_i1_i1_(param_4, param_5);
      p = x_323;
      int x_324 = p;
      int x_326 = l_1;
      if (((x_324 - 1) > x_326)) {
        int x_330 = top;
        int x_331 = (x_330 + 1);
        top = x_331;
        int x_332 = l_1;
        stack[x_331] = x_332;
        int x_334 = top;
        int x_335 = (x_334 + 1);
        top = x_335;
        int x_336 = p;
        stack[x_335] = (x_336 - 1);
      }
      int x_339 = p;
      int x_341 = h_1;
      if (((x_339 + 1) < x_341)) {
        int x_345 = top;
        int x_346 = (x_345 + 1);
        top = x_346;
        int x_347 = p;
        stack[x_346] = (x_347 + 1);
        int x_350 = top;
        int x_351 = (x_350 + 1);
        top = x_351;
        int x_352 = h_1;
        stack[x_351] = x_352;
      }
      {
      }
      continue;
    }
  }
}
void main_1() {
  int i_2 = 0;
  vec2 uv = vec2(0.0f);
  vec3 color = vec3(0.0f);
  vec4 x_94 = x_GLF_pos;
  x_GLF_FragCoord = ((x_94 + vec4(1.0f, 1.0f, 0.0f, 0.0f)) * vec4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      int x_101 = i_2;
      if ((x_101 < 10)) {
      } else {
        break;
      }
      int x_104 = i_2;
      int x_105 = i_2;
      obj.numbers[x_104] = (10 - x_105);
      float x_109 = x_33.injectionSwitch.x;
      float x_111 = x_33.injectionSwitch.y;
      if ((x_109 > x_111)) {
        break;
      }
      int x_115 = i_2;
      int x_116 = i_2;
      int x_118 = obj.numbers[x_116];
      int x_119 = i_2;
      int x_121 = obj.numbers[x_119];
      obj.numbers[x_115] = (x_118 * x_121);
      {
        int x_124 = i_2;
        i_2 = (x_124 + 1);
      }
      continue;
    }
  }
  quicksort_();
  vec4 x_127 = x_GLF_FragCoord;
  vec2 x_130 = x_36.resolution;
  uv = (vec2(x_127[0u], x_127[1u]) / x_130);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_133 = obj.numbers[0];
  float x_136 = color.x;
  color[0u] = (x_136 + float(x_133));
  float x_140 = uv.x;
  if ((x_140 > 0.25f)) {
    int x_145 = obj.numbers[1];
    float x_148 = color.x;
    color[0u] = (x_148 + float(x_145));
  }
  float x_152 = uv.x;
  if ((x_152 > 0.5f)) {
    int x_157 = obj.numbers[2];
    float x_160 = color.y;
    color[1u] = (x_160 + float(x_157));
  }
  float x_164 = uv.x;
  if ((x_164 > 0.75f)) {
    int x_169 = obj.numbers[3];
    float x_172 = color.z;
    color[2u] = (x_172 + float(x_169));
  }
  int x_176 = obj.numbers[4];
  float x_179 = color.y;
  color[1u] = (x_179 + float(x_176));
  float x_183 = uv.y;
  if ((x_183 > 0.25f)) {
    int x_188 = obj.numbers[5];
    float x_191 = color.x;
    color[0u] = (x_191 + float(x_188));
  }
  float x_195 = uv.y;
  if ((x_195 > 0.5f)) {
    int x_200 = obj.numbers[6];
    float x_203 = color.y;
    color[1u] = (x_203 + float(x_200));
  }
  float x_207 = uv.y;
  if ((x_207 > 0.75f)) {
    int x_212 = obj.numbers[7];
    float x_215 = color.z;
    color[2u] = (x_215 + float(x_212));
  }
  int x_219 = obj.numbers[8];
  float x_222 = color.z;
  color[2u] = (x_222 + float(x_219));
  float x_226 = uv.x;
  float x_228 = uv.y;
  if ((abs((x_226 - x_228)) < 0.25f)) {
    int x_235 = obj.numbers[9];
    float x_238 = color.x;
    color[0u] = (x_238 + float(x_235));
  }
  vec3 x_241 = color;
  vec3 x_242 = normalize(x_241);
  frag_color = vec4(x_242[0u], x_242[1u], x_242[2u], 1.0f);
  vec4 x_247 = x_GLF_pos;
  tint_symbol = x_247;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:264: 'main' : function cannot take any parameter(s) 
ERROR: 0:264: 'structure' :  entry point cannot return a value
ERROR: 0:264: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
