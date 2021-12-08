SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 x_GLF_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_34 : register(b0, space0) {
  uint4 x_34[1];
};
static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

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
  int x_103[10] = (int[10])0;
  int x_104 = 0;
  int x_105 = 0;
  int x_106 = 0;
  int i_2 = 0;
  float2 uv = float2(0.0f, 0.0f);
  float3 color = float3(0.0f, 0.0f, 0.0f);
  x_GLF_FragCoord = ((x_GLF_pos + float4(1.0f, 1.0f, 0.0f, 0.0f)) * float4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    [loop] for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      obj.numbers[i_2] = (10 - i_2);
      const int x_121 = i_2;
      const int x_124 = obj.numbers[i_2];
      const int x_127 = obj.numbers[i_2];
      obj.numbers[x_121] = (x_124 * x_127);
    }
  }
  x_100 = 0;
  x_101 = 9;
  x_102 = -1;
  const int x_133 = (x_102 + 1);
  x_102 = x_133;
  x_103[x_133] = x_100;
  const int x_137 = (x_102 + 1);
  x_102 = x_137;
  x_103[x_137] = x_101;
  [loop] while (true) {
    if ((x_102 >= 0)) {
    } else {
      break;
    }
    const int x_147 = x_102;
    x_102 = (x_147 - 1);
    const int x_150 = x_103[x_147];
    x_101 = x_150;
    const int x_151 = x_102;
    x_102 = (x_151 - 1);
    const int x_154 = x_103[x_151];
    x_100 = x_154;
    x_105 = x_100;
    x_106 = x_101;
    const int x_159 = obj.numbers[x_106];
    x_92 = x_159;
    x_93 = (x_105 - 1);
    x_94 = x_105;
    {
      [loop] for(; (x_94 <= (x_106 - 1)); x_94 = (x_94 + 1)) {
        const int x_174 = obj.numbers[x_94];
        if ((x_174 <= x_92)) {
          x_93 = (x_93 + 1);
          x_95 = x_93;
          x_96 = x_94;
          const int x_185 = obj.numbers[x_95];
          x_91 = x_185;
          const int x_186 = x_95;
          const int x_189 = obj.numbers[x_96];
          obj.numbers[x_186] = x_189;
          obj.numbers[x_96] = x_91;
        }
      }
    }
    x_97 = (x_93 + 1);
    x_98 = x_106;
    const int x_201 = obj.numbers[x_97];
    x_90 = x_201;
    const int x_202 = x_97;
    const int x_205 = obj.numbers[x_98];
    obj.numbers[x_202] = x_205;
    obj.numbers[x_98] = x_90;
    x_99 = (x_93 + 1);
    x_104 = x_99;
    if (((x_104 - 1) > x_100)) {
      const int x_220 = (x_102 + 1);
      x_102 = x_220;
      x_103[x_220] = x_100;
      const int x_224 = (x_102 + 1);
      x_102 = x_224;
      x_103[x_224] = (x_104 - 1);
    }
    if (((x_104 + 1) < x_101)) {
      const int x_235 = (x_102 + 1);
      x_102 = x_235;
      x_103[x_235] = (x_104 + 1);
      const int x_240 = (x_102 + 1);
      x_102 = x_240;
      x_103[x_240] = x_101;
    }
  }
  const float4 x_243 = x_GLF_FragCoord;
  const float2 x_246 = asfloat(x_34[0].xy);
  uv = (float2(x_243.x, x_243.y) / x_246);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_249 = obj.numbers[0];
  const float x_252 = color.x;
  color.x = (x_252 + float(x_249));
  const float x_256 = uv.x;
  if ((x_256 > 0.25f)) {
    const int x_261 = obj.numbers[1];
    const float x_264 = color.x;
    color.x = (x_264 + float(x_261));
  }
  const float x_268 = uv.x;
  if ((x_268 > 0.5f)) {
    const int x_273 = obj.numbers[2];
    const float x_276 = color.y;
    color.y = (x_276 + float(x_273));
  }
  const float x_280 = uv.x;
  if ((x_280 > 0.75f)) {
    const int x_285 = obj.numbers[3];
    const float x_288 = color.z;
    color.z = (x_288 + float(x_285));
  }
  const int x_292 = obj.numbers[4];
  const float x_295 = color.y;
  color.y = (x_295 + float(x_292));
  const float x_299 = uv.y;
  if ((x_299 > 0.25f)) {
    const int x_304 = obj.numbers[5];
    const float x_307 = color.x;
    color.x = (x_307 + float(x_304));
  }
  const float x_311 = uv.y;
  if ((x_311 > 0.5f)) {
    const int x_316 = obj.numbers[6];
    const float x_319 = color.y;
    color.y = (x_319 + float(x_316));
  }
  const float x_323 = uv.y;
  if ((x_323 > 0.75f)) {
    const int x_328 = obj.numbers[7];
    const float x_331 = color.z;
    color.z = (x_331 + float(x_328));
  }
  const int x_335 = obj.numbers[8];
  const float x_338 = color.z;
  color.z = (x_338 + float(x_335));
  const float x_342 = uv.x;
  const float x_344 = uv.y;
  if ((abs((x_342 - x_344)) < 0.25f)) {
    const int x_351 = obj.numbers[9];
    const float x_354 = color.x;
    color.x = (x_354 + float(x_351));
  }
  const float3 x_358 = normalize(color);
  frag_color = float4(x_358.x, x_358.y, x_358.z, 1.0f);
  gl_Position = x_GLF_pos;
  return;
}

struct main_out {
  float4 frag_color_1;
  float4 gl_Position;
};
struct tint_symbol_1 {
  float4 x_GLF_pos_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 frag_color_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

main_out main_inner(float4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  const main_out tint_symbol_4 = {frag_color, gl_Position};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_GLF_pos_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.frag_color_1 = inner_result.frag_color_1;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_366 = i;
  const int x_368 = obj.numbers[x_366];
  temp = x_368;
  const int x_369 = i;
  const int x_370 = j;
  const int x_372 = obj.numbers[x_370];
  obj.numbers[x_369] = x_372;
  const int x_374 = j;
  obj.numbers[x_374] = temp;
  return;
}

int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  const int x_378 = h;
  const int x_380 = obj.numbers[x_378];
  pivot = x_380;
  const int x_381 = l;
  i_1 = (x_381 - 1);
  const int x_383 = l;
  j_1 = x_383;
  [loop] while (true) {
    const int x_388 = j_1;
    const int x_389 = h;
    if ((x_388 <= (x_389 - 1))) {
    } else {
      break;
    }
    const int x_395 = obj.numbers[j_1];
    if ((x_395 <= pivot)) {
      i_1 = (i_1 + 1);
      param = i_1;
      param_1 = j_1;
      swap_i1_i1_(param, param_1);
    }
    {
      j_1 = (j_1 + 1);
    }
  }
  param_2 = (i_1 + 1);
  const int x_409 = h;
  param_3 = x_409;
  swap_i1_i1_(param_2, param_3);
  return (i_1 + 1);
}

void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = (int[10])0;
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  const int x_415 = (top + 1);
  top = x_415;
  stack[x_415] = l_1;
  const int x_419 = (top + 1);
  top = x_419;
  stack[x_419] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_429 = top;
    top = (x_429 - 1);
    const int x_432 = stack[x_429];
    h_1 = x_432;
    const int x_433 = top;
    top = (x_433 - 1);
    const int x_436 = stack[x_433];
    l_1 = x_436;
    param_4 = l_1;
    param_5 = h_1;
    const int x_439 = performPartition_i1_i1_(param_4, param_5);
    p = x_439;
    if (((p - 1) > l_1)) {
      const int x_447 = (top + 1);
      top = x_447;
      stack[x_447] = l_1;
      const int x_451 = (top + 1);
      top = x_451;
      stack[x_451] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_462 = (top + 1);
      top = x_462;
      stack[x_462] = (p + 1);
      const int x_467 = (top + 1);
      top = x_467;
      stack[x_467] = h_1;
    }
  }
  return;
}
C:\src\tint\test\Shader@0x0000028832C83FA0(39,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x0000028832C83FA0(38,12-45): error X3531: can't unroll loops marked with loop attribute

