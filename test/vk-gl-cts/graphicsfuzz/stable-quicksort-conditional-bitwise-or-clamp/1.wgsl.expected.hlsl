SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_34 : register(b0, space0) {
  uint4 x_34[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_230 = i;
  const int x_232 = obj.numbers[x_230];
  temp = x_232;
  const int x_233 = i;
  const int x_234 = j;
  const int x_236 = obj.numbers[x_234];
  obj.numbers[x_233] = x_236;
  const int x_238 = j;
  obj.numbers[x_238] = temp;
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
  const int x_242 = h;
  const int x_244 = obj.numbers[x_242];
  pivot = x_244;
  const int x_245 = l;
  i_1 = (x_245 - 1);
  const int x_247 = l;
  j_1 = x_247;
  [loop] while (true) {
    const int x_252 = j_1;
    const int x_253 = h;
    if ((x_252 <= (x_253 - 1))) {
    } else {
      break;
    }
    const int x_259 = obj.numbers[j_1];
    if ((x_259 <= pivot)) {
      i_1 = (i_1 + 1);
      param = i_1;
      param_1 = j_1;
      swap_i1_i1_(param, param_1);
    }
    {
      j_1 = (j_1 + 1);
    }
  }
  i_1 = (i_1 + 1);
  param_2 = i_1;
  const int x_274 = h;
  param_3 = x_274;
  swap_i1_i1_(param_2, param_3);
  return i_1;
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
  const int x_281 = (top + 1);
  top = x_281;
  stack[x_281] = l_1;
  const float x_285 = gl_FragCoord.y;
  if ((x_285 >= 0.0f)) {
    const int x_290 = h_1;
    if (false) {
      x_279 = 1;
    } else {
      x_279 = (h_1 << asuint(0));
    }
    x_278 = (x_290 | x_279);
  } else {
    x_278 = 1;
  }
  int_a = x_278;
  clamp_a = clamp(h_1, h_1, int_a);
  const int x_304 = (top + 1);
  top = x_304;
  stack[x_304] = (clamp_a / 1);
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_315 = top;
    top = (x_315 - 1);
    const int x_318 = stack[x_315];
    h_1 = x_318;
    const int x_319 = top;
    top = (x_319 - 1);
    const int x_322 = stack[x_319];
    l_1 = x_322;
    param_4 = l_1;
    param_5 = h_1;
    const int x_325 = performPartition_i1_i1_(param_4, param_5);
    p = x_325;
    if (((p - 1) > l_1)) {
      const int x_333 = (top + 1);
      top = x_333;
      stack[x_333] = l_1;
      const int x_337 = (top + 1);
      top = x_337;
      stack[x_337] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_348 = (top + 1);
      top = x_348;
      stack[x_348] = (p + 1);
      const int x_353 = (top + 1);
      top = x_353;
      stack[x_353] = h_1;
    }
  }
  return;
}

void main_1() {
  int i_2 = 0;
  float2 uv = float2(0.0f, 0.0f);
  float3 color = float3(0.0f, 0.0f, 0.0f);
  i_2 = 0;
  {
    [loop] for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      obj.numbers[i_2] = (10 - i_2);
      const int x_97 = i_2;
      const int x_100 = obj.numbers[i_2];
      const int x_103 = obj.numbers[i_2];
      obj.numbers[x_97] = (x_100 * x_103);
    }
  }
  quicksort_();
  const float4 x_109 = gl_FragCoord;
  const float2 x_112 = asfloat(x_34[0].xy);
  uv = (float2(x_109.x, x_109.y) / x_112);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_115 = obj.numbers[0];
  const float x_118 = color.x;
  color.x = (x_118 + float(x_115));
  const float x_122 = uv.x;
  if ((x_122 > 0.25f)) {
    const int x_127 = obj.numbers[1];
    const float x_130 = color.x;
    color.x = (x_130 + float(x_127));
  }
  const float x_134 = uv.x;
  if ((x_134 > 0.5f)) {
    const int x_139 = obj.numbers[2];
    const float x_142 = color.y;
    color.y = (x_142 + float(x_139));
  }
  const float x_146 = uv.x;
  if ((x_146 > 0.75f)) {
    const int x_151 = obj.numbers[3];
    const float x_154 = color.z;
    color.z = (x_154 + float(x_151));
  }
  const int x_158 = obj.numbers[4];
  const float x_161 = color.y;
  color.y = (x_161 + float(x_158));
  const float x_165 = uv.y;
  if ((x_165 > 0.25f)) {
    const int x_170 = obj.numbers[5];
    const float x_173 = color.x;
    color.x = (x_173 + float(x_170));
  }
  const float x_177 = uv.y;
  if ((x_177 > 0.5f)) {
    const int x_182 = obj.numbers[6];
    const float x_185 = color.y;
    color.y = (x_185 + float(x_182));
  }
  const float x_189 = uv.y;
  if ((x_189 > 0.75f)) {
    const int x_194 = obj.numbers[7];
    const float x_197 = color.z;
    color.z = (x_197 + float(x_194));
  }
  const int x_201 = obj.numbers[8];
  const float x_204 = color.z;
  color.z = (x_204 + float(x_201));
  const float x_208 = uv.x;
  const float x_210 = uv.y;
  if ((abs((x_208 - x_210)) < 0.25f)) {
    const int x_217 = obj.numbers[9];
    const float x_220 = color.x;
    color.x = (x_220 + float(x_217));
  }
  const float3 x_224 = normalize(color);
  x_GLF_color = float4(x_224.x, x_224.y, x_224.z, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x000001EE8601E080(146,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x000001EE8601E080(145,12-45): error X3531: can't unroll loops marked with loop attribute

