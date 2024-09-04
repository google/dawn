SKIP: FAILED

#version 310 es

struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_32;
vec4 x_GLF_color = vec4(0.0f);
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  int x_225 = i;
  int x_227 = obj.numbers[x_225];
  temp = x_227;
  int x_228 = i;
  int x_229 = j;
  int x_231 = obj.numbers[x_229];
  obj.numbers[x_228] = x_231;
  int x_233 = j;
  int x_234 = temp;
  obj.numbers[x_233] = x_234;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_237 = h;
  int x_239 = obj.numbers[x_237];
  pivot = x_239;
  int x_240 = l;
  i_1 = (x_240 - 1);
  int x_242 = l;
  j_1 = x_242;
  {
    while(true) {
      int x_247 = j_1;
      int x_248 = h;
      if ((x_247 <= (x_248 - 1))) {
      } else {
        break;
      }
      int x_252 = j_1;
      int x_254 = obj.numbers[x_252];
      int x_255 = pivot;
      if ((x_254 <= x_255)) {
        int x_259 = i_1;
        i_1 = (x_259 + 1);
        int x_261 = i_1;
        param = x_261;
        int x_262 = j_1;
        param_1 = x_262;
        swap_i1_i1_(param, param_1);
      }
      {
        int x_264 = j_1;
        j_1 = (x_264 + 1);
      }
      continue;
    }
  }
  int x_266 = i_1;
  i_1 = (x_266 + 1);
  int x_268 = i_1;
  param_2 = x_268;
  int x_269 = h;
  param_3 = x_269;
  swap_i1_i1_(param_2, param_3);
  int x_271 = i_1;
  return x_271;
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
  int x_273 = top;
  int x_274 = (x_273 + 1);
  top = x_274;
  int x_275 = l_1;
  stack[x_274] = x_275;
  int x_277 = top;
  int x_278 = (x_277 + 1);
  top = x_278;
  int x_279 = h_1;
  stack[x_278] = x_279;
  {
    while(true) {
      int x_285 = top;
      if ((x_285 >= 0)) {
      } else {
        break;
      }
      int x_288 = top;
      top = (x_288 - 1);
      int x_291 = stack[x_288];
      h_1 = x_291;
      int x_292 = top;
      top = (x_292 - 1);
      int x_295 = stack[x_292];
      l_1 = x_295;
      int x_296 = l_1;
      param_4 = x_296;
      int x_297 = h_1;
      param_5 = x_297;
      int x_298 = performPartition_i1_i1_(param_4, param_5);
      p = x_298;
      int x_299 = p;
      int x_301 = l_1;
      if (((x_299 - 1) > x_301)) {
        int x_305 = top;
        int x_306 = (x_305 + 1);
        top = x_306;
        int x_307 = l_1;
        stack[x_306] = x_307;
        int x_309 = top;
        int x_310 = (x_309 + 1);
        top = x_310;
        int x_311 = p;
        stack[x_310] = (x_311 - 1);
      }
      int x_314 = p;
      int x_316 = h_1;
      if (((x_314 + 1) < x_316)) {
        int x_320 = top;
        int x_321 = (x_320 + 1);
        top = x_321;
        int x_322 = p;
        stack[x_321] = (x_322 + 1);
        int x_325 = top;
        int x_326 = (x_325 + 1);
        top = x_326;
        int x_327 = h_1;
        stack[x_326] = x_327;
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
  i_2 = 0;
  {
    while(true) {
      int x_85 = i_2;
      if ((x_85 < 10)) {
      } else {
        break;
      }
      int x_88 = i_2;
      int x_89 = i_2;
      obj.numbers[x_88] = (10 - x_89);
      int x_92 = i_2;
      int x_93 = i_2;
      int x_95 = obj.numbers[x_93];
      int x_96 = i_2;
      int x_98 = obj.numbers[x_96];
      obj.numbers[x_92] = (x_95 * x_98);
      {
        int x_101 = i_2;
        i_2 = (x_101 + 1);
      }
      continue;
    }
  }
  quicksort_();
  vec4 x_104 = tint_symbol;
  vec2 x_107 = x_32.resolution;
  uv = (vec2(x_104[0u], x_104[1u]) / x_107);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_110 = obj.numbers[0];
  float x_113 = color.x;
  color[0u] = (x_113 + float(x_110));
  float x_117 = uv.x;
  if ((x_117 > 0.25f)) {
    int x_122 = obj.numbers[1];
    float x_125 = color.x;
    color[0u] = (x_125 + float(x_122));
  }
  float x_129 = uv.x;
  if ((x_129 > 0.5f)) {
    int x_134 = obj.numbers[2];
    float x_137 = color.y;
    color[1u] = (x_137 + float(x_134));
  }
  float x_141 = uv.x;
  if ((x_141 > 0.75f)) {
    int x_146 = obj.numbers[3];
    float x_149 = color.z;
    color[2u] = (x_149 + float(x_146));
  }
  int x_153 = obj.numbers[4];
  float x_156 = color.y;
  color[1u] = (x_156 + float(x_153));
  float x_160 = uv.y;
  if ((x_160 > 0.25f)) {
    int x_165 = obj.numbers[5];
    float x_168 = color.x;
    color[0u] = (x_168 + float(x_165));
  }
  float x_172 = uv.y;
  if ((x_172 > 0.5f)) {
    int x_177 = obj.numbers[6];
    float x_180 = color.y;
    color[1u] = (x_180 + float(x_177));
  }
  float x_184 = uv.y;
  if ((x_184 > 0.75f)) {
    int x_189 = obj.numbers[7];
    float x_192 = color.z;
    color[2u] = (x_192 + float(x_189));
  }
  int x_196 = obj.numbers[8];
  float x_199 = color.z;
  color[2u] = (x_199 + float(x_196));
  float x_203 = uv.x;
  float x_205 = uv.y;
  if ((abs((x_203 - x_205)) < 0.25f)) {
    int x_212 = obj.numbers[9];
    float x_215 = color.x;
    color[0u] = (x_215 + float(x_212));
  }
  vec3 x_218 = color;
  vec3 x_219 = normalize(x_218);
  x_GLF_color = vec4(x_219[0u], x_219[1u], x_219[2u], 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
