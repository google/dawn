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
  const int x_316 = h;
  const int x_318 = obj.numbers[x_316];
  pivot = x_318;
  const int x_319 = l;
  i_1 = (x_319 - 1);
  const int x_321 = l;
  j_1 = x_321;
  [loop] while (true) {
    const int x_326 = j_1;
    const int x_327 = h;
    if ((x_326 <= (x_327 - 1))) {
    } else {
      break;
    }
    const int x_333 = obj.numbers[j_1];
    if ((x_333 <= pivot)) {
      i_1 = (i_1 + 1);
      param = i_1;
      param_1 = j_1;
      const int x_344 = obj.numbers[param];
      x_315 = x_344;
      const int x_345 = param;
      const int x_348 = obj.numbers[param_1];
      obj.numbers[x_345] = x_348;
      obj.numbers[param_1] = x_315;
    }
    {
      j_1 = (j_1 + 1);
    }
  }
  param_2 = (i_1 + 1);
  const int x_357 = h;
  param_3 = x_357;
  const int x_360 = obj.numbers[param_2];
  x_314 = x_360;
  const int x_361 = param_2;
  const int x_364 = obj.numbers[param_3];
  obj.numbers[x_361] = x_364;
  obj.numbers[param_3] = x_314;
  if (false) {
  } else {
    return (i_1 + 1);
  }
  return 0;
}

void main_1() {
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94[10] = (int[10])0;
  int x_95 = 0;
  int x_96 = 0;
  int x_97 = 0;
  int i_2 = 0;
  float2 uv = float2(0.0f, 0.0f);
  float3 color = float3(0.0f, 0.0f, 0.0f);
  x_GLF_FragCoord = ((x_GLF_pos + float4(1.0f, 1.0f, 0.0f, 0.0f)) * float4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    [loop] for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      obj.numbers[i_2] = (10 - i_2);
      const int x_112 = i_2;
      const int x_115 = obj.numbers[i_2];
      const int x_118 = obj.numbers[i_2];
      obj.numbers[x_112] = (x_115 * x_118);
    }
  }
  x_91 = 0;
  x_92 = 9;
  x_93 = -1;
  const int x_124 = (x_93 + 1);
  x_93 = x_124;
  x_94[x_124] = x_91;
  const int x_128 = (x_93 + 1);
  x_93 = x_128;
  x_94[x_128] = x_92;
  [loop] while (true) {
    if ((x_93 >= 0)) {
    } else {
      break;
    }
    const int x_138 = x_93;
    x_93 = (x_138 - 1);
    const int x_141 = x_94[x_138];
    x_92 = x_141;
    const int x_142 = x_93;
    x_93 = (x_142 - 1);
    const int x_145 = x_94[x_142];
    x_91 = x_145;
    x_96 = x_91;
    x_97 = x_92;
    const int x_148 = performPartition_i1_i1_(x_96, x_97);
    x_95 = x_148;
    if (((x_95 - 1) > x_91)) {
      const int x_156 = (x_93 + 1);
      x_93 = x_156;
      x_94[x_156] = x_91;
      const int x_160 = (x_93 + 1);
      x_93 = x_160;
      x_94[x_160] = (x_95 - 1);
    }
    if (((x_95 + 1) < x_92)) {
      const int x_171 = (x_93 + 1);
      x_93 = x_171;
      x_94[x_171] = (x_95 + 1);
      const int x_176 = (x_93 + 1);
      x_93 = x_176;
      x_94[x_176] = x_92;
    }
  }
  const float4 x_179 = x_GLF_FragCoord;
  const float2 x_182 = asfloat(x_34[0].xy);
  uv = (float2(x_179.x, x_179.y) / x_182);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_185 = obj.numbers[0];
  const float x_188 = color.x;
  color.x = (x_188 + float(x_185));
  const float x_192 = uv.x;
  if ((x_192 > 0.25f)) {
    const int x_197 = obj.numbers[1];
    const float x_200 = color.x;
    color.x = (x_200 + float(x_197));
  }
  const float x_204 = uv.x;
  if ((x_204 > 0.5f)) {
    const int x_209 = obj.numbers[2];
    const float x_212 = color.y;
    color.y = (x_212 + float(x_209));
  }
  const float x_216 = uv.x;
  if ((x_216 > 0.75f)) {
    const int x_221 = obj.numbers[3];
    const float x_224 = color.z;
    color.z = (x_224 + float(x_221));
  }
  const int x_228 = obj.numbers[4];
  const float x_231 = color.y;
  color.y = (x_231 + float(x_228));
  const float x_235 = uv.y;
  if ((x_235 > 0.25f)) {
    const int x_240 = obj.numbers[5];
    const float x_243 = color.x;
    color.x = (x_243 + float(x_240));
  }
  const float x_247 = uv.y;
  if ((x_247 > 0.5f)) {
    const int x_252 = obj.numbers[6];
    const float x_255 = color.y;
    color.y = (x_255 + float(x_252));
  }
  const float x_259 = uv.y;
  if ((x_259 > 0.75f)) {
    const int x_264 = obj.numbers[7];
    const float x_267 = color.z;
    color.z = (x_267 + float(x_264));
  }
  const int x_271 = obj.numbers[8];
  const float x_274 = color.z;
  color.z = (x_274 + float(x_271));
  const float x_278 = uv.x;
  const float x_280 = uv.y;
  if ((abs((x_278 - x_280)) < 0.25f)) {
    const int x_287 = obj.numbers[9];
    const float x_290 = color.x;
    color.x = (x_290 + float(x_287));
  }
  const float3 x_294 = normalize(color);
  frag_color = float4(x_294.x, x_294.y, x_294.z, 1.0f);
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
  const int x_302 = i;
  const int x_304 = obj.numbers[x_302];
  temp = x_304;
  const int x_305 = i;
  const int x_306 = j;
  const int x_308 = obj.numbers[x_306];
  obj.numbers[x_305] = x_308;
  const int x_310 = j;
  obj.numbers[x_310] = temp;
  return;
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
  const int x_377 = (top + 1);
  top = x_377;
  stack[x_377] = l_1;
  const int x_381 = (top + 1);
  top = x_381;
  stack[x_381] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_391 = top;
    top = (x_391 - 1);
    const int x_394 = stack[x_391];
    h_1 = x_394;
    const int x_395 = top;
    top = (x_395 - 1);
    const int x_398 = stack[x_395];
    l_1 = x_398;
    param_4 = l_1;
    param_5 = h_1;
    const int x_401 = performPartition_i1_i1_(param_4, param_5);
    p = x_401;
    if (((p - 1) > l_1)) {
      const int x_409 = (top + 1);
      top = x_409;
      stack[x_409] = l_1;
      const int x_413 = (top + 1);
      top = x_413;
      stack[x_413] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_424 = (top + 1);
      top = x_424;
      stack[x_424] = (p + 1);
      const int x_429 = (top + 1);
      top = x_429;
      stack[x_429] = h_1;
    }
  }
  return;
}
C:\src\tint\test\Shader@0x000001381E0F76E0(85,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x000001381E0F76E0(84,12-45): error X3531: can't unroll loops marked with loop attribute

