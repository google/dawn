SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_32 : register(b0, space0) {
  uint4 x_32[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void swap_i1_i1_(inout int i, inout int j, float3x3 x_228) {
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
      swap_i1_i1_(param, param_1, float3x3(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f)));
    }
    {
      j_1 = (j_1 + 1);
    }
  }
  i_1 = (i_1 + 1);
  param_2 = i_1;
  const int x_274 = h;
  param_3 = x_274;
  swap_i1_i1_(param_2, param_3, float3x3(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f)));
  return i_1;
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
  const int x_279 = (top + 1);
  top = x_279;
  stack[x_279] = l_1;
  const int x_283 = (top + 1);
  top = x_283;
  stack[x_283] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_293 = top;
    top = (x_293 - 1);
    const int x_296 = stack[x_293];
    h_1 = x_296;
    const int x_297 = top;
    top = (x_297 - 1);
    const int x_300 = stack[x_297];
    l_1 = x_300;
    param_4 = l_1;
    param_5 = h_1;
    const int x_303 = performPartition_i1_i1_(param_4, param_5);
    p = x_303;
    if (((p - 1) > l_1)) {
      const int x_311 = (top + 1);
      top = x_311;
      stack[x_311] = l_1;
      const int x_315 = (top + 1);
      top = x_315;
      stack[x_315] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_326 = (top + 1);
      top = x_326;
      stack[x_326] = (p + 1);
      const int x_331 = (top + 1);
      top = x_331;
      stack[x_331] = h_1;
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
      const int x_96 = i_2;
      const int x_99 = obj.numbers[i_2];
      const int x_102 = obj.numbers[i_2];
      obj.numbers[x_96] = (x_99 * x_102);
    }
  }
  quicksort_();
  const float4 x_108 = gl_FragCoord;
  const float2 x_111 = asfloat(x_32[0].xy);
  uv = (float2(x_108.x, x_108.y) / x_111);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_114 = obj.numbers[0];
  const float x_117 = color.x;
  color.x = (x_117 + float(x_114));
  const float x_121 = uv.x;
  if ((x_121 > 0.25f)) {
    const int x_126 = obj.numbers[1];
    const float x_129 = color.x;
    color.x = (x_129 + float(x_126));
  }
  const float x_133 = uv.x;
  if ((x_133 > 0.5f)) {
    const int x_138 = obj.numbers[2];
    const float x_141 = color.y;
    color.y = (x_141 + float(x_138));
  }
  const float x_145 = uv.x;
  if ((x_145 > 0.75f)) {
    const int x_150 = obj.numbers[3];
    const float x_153 = color.z;
    color.z = (x_153 + float(x_150));
  }
  const int x_157 = obj.numbers[4];
  const float x_160 = color.y;
  color.y = (x_160 + float(x_157));
  const float x_164 = uv.y;
  if ((x_164 > 0.25f)) {
    const int x_169 = obj.numbers[5];
    const float x_172 = color.x;
    color.x = (x_172 + float(x_169));
  }
  const float x_176 = uv.y;
  if ((x_176 > 0.5f)) {
    const int x_181 = obj.numbers[6];
    const float x_184 = color.y;
    color.y = (x_184 + float(x_181));
  }
  const float x_188 = uv.y;
  if ((x_188 > 0.75f)) {
    const int x_193 = obj.numbers[7];
    const float x_196 = color.z;
    color.z = (x_196 + float(x_193));
  }
  const int x_200 = obj.numbers[8];
  const float x_203 = color.z;
  color.z = (x_203 + float(x_200));
  const float x_207 = uv.x;
  const float x_209 = uv.y;
  if ((abs((x_207 - x_209)) < 0.25f)) {
    const int x_216 = obj.numbers[9];
    const float x_219 = color.x;
    color.x = (x_219 + float(x_216));
  }
  const float3 x_223 = normalize(color);
  x_GLF_color = float4(x_223.x, x_223.y, x_223.z, 1.0f);
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
C:\src\tint\test\Shader@0x0000029339D00EC0(128,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x0000029339D00EC0(127,12-45): error X3531: can't unroll loops marked with loop attribute

