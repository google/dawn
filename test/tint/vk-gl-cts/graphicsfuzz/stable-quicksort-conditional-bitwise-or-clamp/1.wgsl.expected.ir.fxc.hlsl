SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


static QuicksortObject obj = (QuicksortObject)0;
static float4 gl_FragCoord = (0.0f).xxxx;
cbuffer cbuffer_x_34 : register(b0) {
  uint4 x_34[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void swap_i1_i1_(inout int i, inout int j) {
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
        swap_i1_i1_(param, param_1);
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
  swap_i1_i1_(param_2, param_3);
  int x_276 = i_1;
  return x_276;
}

int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs)));
}

void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = (int[10])0;
  int int_a = 0;
  int x_278 = 0;
  int x_279 = 0;
  int clamp_a = 0;
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  int x_280 = top;
  int x_281 = (x_280 + 1);
  top = x_281;
  int x_282 = l_1;
  stack[x_281] = x_282;
  float x_285 = gl_FragCoord.y;
  if ((x_285 >= 0.0f)) {
    int x_290 = h_1;
    if (false) {
      x_279 = 1;
    } else {
      int x_294 = h_1;
      x_279 = (x_294 << (0u & 31u));
    }
    int x_296 = x_279;
    x_278 = (x_290 | x_296);
  } else {
    x_278 = 1;
  }
  int x_298 = x_278;
  int_a = x_298;
  int x_299 = h_1;
  int x_300 = h_1;
  int x_301 = int_a;
  clamp_a = min(max(x_299, x_300), x_301);
  int x_303 = top;
  int x_304 = (x_303 + 1);
  top = x_304;
  int x_305 = clamp_a;
  stack[x_304] = tint_div_i32(x_305, 1);
  {
    while(true) {
      int x_312 = top;
      if ((x_312 >= 0)) {
      } else {
        break;
      }
      int x_315 = top;
      top = (x_315 - 1);
      int x_318 = stack[x_315];
      h_1 = x_318;
      int x_319 = top;
      top = (x_319 - 1);
      int x_322 = stack[x_319];
      l_1 = x_322;
      int x_323 = l_1;
      param_4 = x_323;
      int x_324 = h_1;
      param_5 = x_324;
      int x_325 = performPartition_i1_i1_(param_4, param_5);
      p = x_325;
      int x_326 = p;
      int x_328 = l_1;
      if (((x_326 - 1) > x_328)) {
        int x_332 = top;
        int x_333 = (x_332 + 1);
        top = x_333;
        int x_334 = l_1;
        stack[x_333] = x_334;
        int x_336 = top;
        int x_337 = (x_336 + 1);
        top = x_337;
        int x_338 = p;
        stack[x_337] = (x_338 - 1);
      }
      int x_341 = p;
      int x_343 = h_1;
      if (((x_341 + 1) < x_343)) {
        int x_347 = top;
        int x_348 = (x_347 + 1);
        top = x_348;
        int x_349 = p;
        stack[x_348] = (x_349 + 1);
        int x_352 = top;
        int x_353 = (x_352 + 1);
        top = x_353;
        int x_354 = h_1;
        stack[x_353] = x_354;
      }
      {
      }
      continue;
    }
  }
}

void main_1() {
  int i_2 = 0;
  float2 uv = (0.0f).xx;
  float3 color = (0.0f).xxx;
  i_2 = 0;
  {
    while(true) {
      int x_90 = i_2;
      if ((x_90 < 10)) {
      } else {
        break;
      }
      int x_93 = i_2;
      int x_94 = i_2;
      obj.numbers[x_93] = (10 - x_94);
      int x_97 = i_2;
      int x_98 = i_2;
      int x_100 = obj.numbers[x_98];
      int x_101 = i_2;
      int x_103 = obj.numbers[x_101];
      obj.numbers[x_97] = (x_100 * x_103);
      {
        int x_106 = i_2;
        i_2 = (x_106 + 1);
      }
      continue;
    }
  }
  quicksort_();
  float4 x_109 = gl_FragCoord;
  float2 x_112 = asfloat(x_34[0u].xy);
  uv = (float2(x_109[0u], x_109[1u]) / x_112);
  color = float3(1.0f, 2.0f, 3.0f);
  int x_115 = obj.numbers[0];
  float x_118 = color.x;
  color[0u] = (x_118 + float(x_115));
  float x_122 = uv.x;
  if ((x_122 > 0.25f)) {
    int x_127 = obj.numbers[1];
    float x_130 = color.x;
    color[0u] = (x_130 + float(x_127));
  }
  float x_134 = uv.x;
  if ((x_134 > 0.5f)) {
    int x_139 = obj.numbers[2];
    float x_142 = color.y;
    color[1u] = (x_142 + float(x_139));
  }
  float x_146 = uv.x;
  if ((x_146 > 0.75f)) {
    int x_151 = obj.numbers[3];
    float x_154 = color.z;
    color[2u] = (x_154 + float(x_151));
  }
  int x_158 = obj.numbers[4];
  float x_161 = color.y;
  color[1u] = (x_161 + float(x_158));
  float x_165 = uv.y;
  if ((x_165 > 0.25f)) {
    int x_170 = obj.numbers[5];
    float x_173 = color.x;
    color[0u] = (x_173 + float(x_170));
  }
  float x_177 = uv.y;
  if ((x_177 > 0.5f)) {
    int x_182 = obj.numbers[6];
    float x_185 = color.y;
    color[1u] = (x_185 + float(x_182));
  }
  float x_189 = uv.y;
  if ((x_189 > 0.75f)) {
    int x_194 = obj.numbers[7];
    float x_197 = color.z;
    color[2u] = (x_197 + float(x_194));
  }
  int x_201 = obj.numbers[8];
  float x_204 = color.z;
  color[2u] = (x_204 + float(x_201));
  float x_208 = uv.x;
  float x_210 = uv.y;
  if ((abs((x_208 - x_210)) < 0.25f)) {
    int x_217 = obj.numbers[9];
    float x_220 = color.x;
    color[0u] = (x_220 + float(x_217));
  }
  float3 x_223 = color;
  float3 x_224 = normalize(x_223);
  x_GLF_color = float4(x_224[0u], x_224[1u], x_224[2u], 1.0f);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main(main_inputs inputs) {
  main_out v_1 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(92,11-85): warning X3556: integer divides may be much slower, try using uints if possible.
<scrubbed_path>(32,3-20): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(54,5-15): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(141,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
