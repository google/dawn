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
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  int x_239 = i;
  int x_241 = obj.numbers[x_239];
  temp = x_241;
  int x_242 = i;
  int x_243 = j;
  int x_245 = obj.numbers[x_243];
  obj.numbers[x_242] = x_245;
  int x_247 = j;
  int x_248 = temp;
  obj.numbers[x_247] = x_248;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_251 = h;
  int x_253 = obj.numbers[x_251];
  pivot = x_253;
  int x_254 = l;
  i_1 = (x_254 - 1);
  int x_256 = l;
  j_1 = x_256;
  {
    while(true) {
      int x_261 = j_1;
      int x_262 = h;
      if ((x_261 <= (x_262 - 1))) {
      } else {
        break;
      }
      int x_266 = j_1;
      int x_268 = obj.numbers[x_266];
      int x_269 = pivot;
      if ((x_268 <= x_269)) {
        int x_273 = i_1;
        i_1 = (x_273 + 1);
        int x_275 = i_1;
        param = x_275;
        int x_276 = j_1;
        param_1 = x_276;
        swap_i1_i1_(param, param_1);
      }
      {
        int x_278 = j_1;
        j_1 = (x_278 + 1);
      }
      continue;
    }
  }
  int x_280 = i_1;
  param_2 = (x_280 + 1);
  int x_282 = h;
  param_3 = x_282;
  swap_i1_i1_(param_2, param_3);
  int x_284 = i_1;
  return (x_284 + 1);
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
  int x_287 = top;
  int x_288 = (x_287 + 1);
  top = x_288;
  int x_289 = l_1;
  stack[x_288] = x_289;
  int x_291 = top;
  int x_292 = (x_291 + 1);
  top = x_292;
  int x_293 = h_1;
  stack[x_292] = x_293;
  {
    while(true) {
      int x_299 = top;
      if ((x_299 >= 0)) {
      } else {
        break;
      }
      int x_302 = top;
      top = (x_302 - 1);
      int x_305 = stack[x_302];
      h_1 = x_305;
      int x_306 = top;
      top = (x_306 - 1);
      int x_309 = stack[x_306];
      l_1 = x_309;
      int x_310 = l_1;
      param_4 = x_310;
      int x_311 = h_1;
      param_5 = x_311;
      int x_312 = performPartition_i1_i1_(param_4, param_5);
      p = x_312;
      int x_313 = p;
      int x_315 = l_1;
      if (((x_313 - 1) > x_315)) {
        int x_319 = top;
        int x_320 = (x_319 + 1);
        top = x_320;
        int x_321 = l_1;
        stack[x_320] = x_321;
        int x_323 = top;
        int x_324 = (x_323 + 1);
        top = x_324;
        int x_325 = p;
        stack[x_324] = (x_325 - 1);
      }
      int x_328 = p;
      int x_330 = h_1;
      if (((x_328 + 1) < x_330)) {
        int x_334 = top;
        int x_335 = (x_334 + 1);
        top = x_335;
        int x_336 = p;
        stack[x_335] = (x_336 + 1);
        int x_339 = top;
        int x_340 = (x_339 + 1);
        top = x_340;
        int x_341 = h_1;
        stack[x_340] = x_341;
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
  vec4 x_90 = x_GLF_pos;
  x_GLF_FragCoord = ((x_90 + vec4(1.0f, 1.0f, 0.0f, 0.0f)) * vec4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      int x_97 = i_2;
      if ((x_97 < 10)) {
      } else {
        break;
      }
      int x_100 = i_2;
      int x_101 = i_2;
      obj.numbers[x_100] = (10 - x_101);
      int x_104 = i_2;
      int x_105 = i_2;
      int x_107 = obj.numbers[x_105];
      int x_108 = i_2;
      int x_110 = obj.numbers[x_108];
      obj.numbers[x_104] = (x_107 * x_110);
      {
        int x_113 = i_2;
        i_2 = (x_113 + 1);
      }
      continue;
    }
  }
  quicksort_();
  vec4 x_116 = x_GLF_FragCoord;
  vec2 x_119 = x_34.resolution;
  uv = (vec2(x_116[0u], x_116[1u]) / x_119);
  color = vec3(1.0f, 2.0f, 3.0f);
  int x_122 = obj.numbers[0];
  float x_125 = color.x;
  color[0u] = (x_125 + float(x_122));
  float x_129 = uv.x;
  if ((x_129 > 0.25f)) {
    int x_134 = obj.numbers[1];
    float x_137 = color.x;
    color[0u] = (x_137 + float(x_134));
  }
  float x_141 = uv.x;
  if ((x_141 > 0.5f)) {
    int x_146 = obj.numbers[2];
    float x_149 = color.y;
    color[1u] = (x_149 + float(x_146));
  }
  float x_153 = uv.x;
  if ((x_153 > 0.75f)) {
    int x_158 = obj.numbers[3];
    float x_161 = color.z;
    color[2u] = (x_161 + float(x_158));
  }
  int x_165 = obj.numbers[4];
  float x_168 = color.y;
  color[1u] = (x_168 + float(x_165));
  float x_172 = uv.y;
  if ((x_172 > 0.25f)) {
    int x_177 = obj.numbers[5];
    float x_180 = color.x;
    color[0u] = (x_180 + float(x_177));
  }
  float x_184 = uv.y;
  if ((x_184 > 0.5f)) {
    int x_189 = obj.numbers[6];
    float x_192 = color.y;
    color[1u] = (x_192 + float(x_189));
  }
  float x_196 = uv.y;
  if ((x_196 > 0.75f)) {
    int x_201 = obj.numbers[7];
    float x_204 = color.z;
    color[2u] = (x_204 + float(x_201));
  }
  int x_208 = obj.numbers[8];
  float x_211 = color.z;
  color[2u] = (x_211 + float(x_208));
  float x_215 = uv.x;
  float x_217 = uv.y;
  if ((abs((x_215 - x_217)) < 0.25f)) {
    int x_224 = obj.numbers[9];
    float x_227 = color.x;
    color[0u] = (x_227 + float(x_224));
  }
  vec3 x_230 = color;
  vec3 x_231 = normalize(x_230);
  frag_color = vec4(x_231[0u], x_231[1u], x_231[2u], 1.0f);
  vec4 x_236 = x_GLF_pos;
  tint_symbol = x_236;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:254: 'main' : function cannot take any parameter(s) 
ERROR: 0:254: 'structure' :  entry point cannot return a value
ERROR: 0:254: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
