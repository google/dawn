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
void swap_i1_i1_(inout int i, inout int j, mat3 x_228) {
  int temp = 0;
  int x_230 = i;
  int x_232 = obj.numbers[x_230];
  temp = x_232;
  int x_233 = i;
  int x_234 = j;
  int x_236 = obj.numbers[x_234];
  obj.numbers[x_233] = x_236;
  int x_238 = j;
  int x_239 = temp;
  obj.numbers[x_238] = x_239;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_242 = h;
  int x_244 = obj.numbers[x_242];
  pivot = x_244;
  int x_245 = l;
  i_1 = (x_245 - 1);
  int x_247 = l;
  j_1 = x_247;
  {
    while(true) {
      int x_252 = j_1;
      int x_253 = h;
      if ((x_252 <= (x_253 - 1))) {
      } else {
        break;
      }
      int x_257 = j_1;
      int x_259 = obj.numbers[x_257];
      int x_260 = pivot;
      if ((x_259 <= x_260)) {
        int x_264 = i_1;
        i_1 = (x_264 + 1);
        int x_266 = i_1;
        param = x_266;
        int x_267 = j_1;
        param_1 = x_267;
        swap_i1_i1_(param, param_1, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
      }
      {
        int x_269 = j_1;
        j_1 = (x_269 + 1);
      }
      continue;
    }
  }
  int x_271 = i_1;
  i_1 = (x_271 + 1);
  int x_273 = i_1;
  param_2 = x_273;
  int x_274 = h;
  param_3 = x_274;
  swap_i1_i1_(param_2, param_3, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  int x_276 = i_1;
  return x_276;
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
  int x_278 = top;
  int x_279 = (x_278 + 1);
  top = x_279;
  int x_280 = l_1;
  stack[x_279] = x_280;
  int x_282 = top;
  int x_283 = (x_282 + 1);
  top = x_283;
  int x_284 = h_1;
  stack[x_283] = x_284;
  {
    while(true) {
      int x_290 = top;
      if ((x_290 >= 0)) {
      } else {
        break;
      }
      int x_293 = top;
      top = (x_293 - 1);
      int x_296 = stack[x_293];
      h_1 = x_296;
      int x_297 = top;
      top = (x_297 - 1);
      int x_300 = stack[x_297];
      l_1 = x_300;
      int x_301 = l_1;
      param_4 = x_301;
      int x_302 = h_1;
      param_5 = x_302;
      int x_303 = performPartition_i1_i1_(param_4, param_5);
      p = x_303;
      int x_304 = p;
      int x_306 = l_1;
      if (((x_304 - 1) > x_306)) {
        int x_310 = top;
        int x_311 = (x_310 + 1);
        top = x_311;
        int x_312 = l_1;
        stack[x_311] = x_312;
        int x_314 = top;
        int x_315 = (x_314 + 1);
        top = x_315;
        int x_316 = p;
        stack[x_315] = (x_316 - 1);
      }
      int x_319 = p;
      int x_321 = h_1;
      if (((x_319 + 1) < x_321)) {
        int x_325 = top;
        int x_326 = (x_325 + 1);
        top = x_326;
        int x_327 = p;
        stack[x_326] = (x_327 + 1);
        int x_330 = top;
        int x_331 = (x_330 + 1);
        top = x_331;
        int x_332 = h_1;
        stack[x_331] = x_332;
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
      int x_89 = i_2;
      if ((x_89 < 10)) {
      } else {
        break;
      }
      int x_92 = i_2;
      int x_93 = i_2;
      obj.numbers[x_92] = (10 - x_93);
      int x_96 = i_2;
      int x_97 = i_2;
      int x_99 = obj.numbers[x_97];
      int x_100 = i_2;
      int x_102 = obj.numbers[x_100];
      obj.numbers[x_96] = (x_99 * x_102);
      {
        int x_105 = i_2;
        i_2 = (x_105 + 1);
      }
      continue;
    }
  }
  quicksort_();
  vec4 x_108 = tint_symbol;
  vec2 x_111 = x_32.resolution;
  uv = (vec2(x_108[0u], x_108[1u]) / x_111);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_114 = obj.numbers[0];
  float x_117 = color.x;
  color[0u] = (x_117 + float(x_114));
  float x_121 = uv.x;
  if ((x_121 > 0.25f)) {
    int x_126 = obj.numbers[1];
    float x_129 = color.x;
    color[0u] = (x_129 + float(x_126));
  }
  float x_133 = uv.x;
  if ((x_133 > 0.5f)) {
    int x_138 = obj.numbers[2];
    float x_141 = color.y;
    color[1u] = (x_141 + float(x_138));
  }
  float x_145 = uv.x;
  if ((x_145 > 0.75f)) {
    int x_150 = obj.numbers[3];
    float x_153 = color.z;
    color[2u] = (x_153 + float(x_150));
  }
  int x_157 = obj.numbers[4];
  float x_160 = color.y;
  color[1u] = (x_160 + float(x_157));
  float x_164 = uv.y;
  if ((x_164 > 0.25f)) {
    int x_169 = obj.numbers[5];
    float x_172 = color.x;
    color[0u] = (x_172 + float(x_169));
  }
  float x_176 = uv.y;
  if ((x_176 > 0.5f)) {
    int x_181 = obj.numbers[6];
    float x_184 = color.y;
    color[1u] = (x_184 + float(x_181));
  }
  float x_188 = uv.y;
  if ((x_188 > 0.75f)) {
    int x_193 = obj.numbers[7];
    float x_196 = color.z;
    color[2u] = (x_196 + float(x_193));
  }
  int x_200 = obj.numbers[8];
  float x_203 = color.z;
  color[2u] = (x_203 + float(x_200));
  float x_207 = uv.x;
  float x_209 = uv.y;
  if ((abs((x_207 - x_209)) < 0.25f)) {
    int x_216 = obj.numbers[9];
    float x_219 = color.x;
    color[0u] = (x_219 + float(x_216));
  }
  vec3 x_222 = color;
  vec3 x_223 = normalize(x_222);
  x_GLF_color = vec4(x_223[0u], x_223[1u], x_223[2u], 1.0f);
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
