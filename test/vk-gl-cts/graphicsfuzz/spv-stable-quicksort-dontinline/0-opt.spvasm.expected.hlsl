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

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_225 = i;
  const int x_227 = obj.numbers[x_225];
  temp = x_227;
  const int x_228 = i;
  const int x_229 = j;
  const int x_231 = obj.numbers[x_229];
  obj.numbers[x_228] = x_231;
  const int x_233 = j;
  obj.numbers[x_233] = temp;
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
  const int x_237 = h;
  const int x_239 = obj.numbers[x_237];
  pivot = x_239;
  const int x_240 = l;
  i_1 = (x_240 - 1);
  const int x_242 = l;
  j_1 = x_242;
  [loop] while (true) {
    const int x_247 = j_1;
    const int x_248 = h;
    if ((x_247 <= (x_248 - 1))) {
    } else {
      break;
    }
    const int x_254 = obj.numbers[j_1];
    if ((x_254 <= pivot)) {
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
  const int x_269 = h;
  param_3 = x_269;
  swap_i1_i1_(param_2, param_3);
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
  const int x_274 = (top + 1);
  top = x_274;
  stack[x_274] = l_1;
  const int x_278 = (top + 1);
  top = x_278;
  stack[x_278] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_288 = top;
    top = (x_288 - 1);
    const int x_291 = stack[x_288];
    h_1 = x_291;
    const int x_292 = top;
    top = (x_292 - 1);
    const int x_295 = stack[x_292];
    l_1 = x_295;
    param_4 = l_1;
    param_5 = h_1;
    const int x_298 = performPartition_i1_i1_(param_4, param_5);
    p = x_298;
    if (((p - 1) > l_1)) {
      const int x_306 = (top + 1);
      top = x_306;
      stack[x_306] = l_1;
      const int x_310 = (top + 1);
      top = x_310;
      stack[x_310] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_321 = (top + 1);
      top = x_321;
      stack[x_321] = (p + 1);
      const int x_326 = (top + 1);
      top = x_326;
      stack[x_326] = h_1;
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
      const int x_92 = i_2;
      const int x_95 = obj.numbers[i_2];
      const int x_98 = obj.numbers[i_2];
      obj.numbers[x_92] = (x_95 * x_98);
    }
  }
  quicksort_();
  const float4 x_104 = gl_FragCoord;
  const float2 x_107 = asfloat(x_32[0].xy);
  uv = (float2(x_104.x, x_104.y) / x_107);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_110 = obj.numbers[0];
  const float x_113 = color.x;
  color.x = (x_113 + float(x_110));
  const float x_117 = uv.x;
  if ((x_117 > 0.25f)) {
    const int x_122 = obj.numbers[1];
    const float x_125 = color.x;
    color.x = (x_125 + float(x_122));
  }
  const float x_129 = uv.x;
  if ((x_129 > 0.5f)) {
    const int x_134 = obj.numbers[2];
    const float x_137 = color.y;
    color.y = (x_137 + float(x_134));
  }
  const float x_141 = uv.x;
  if ((x_141 > 0.75f)) {
    const int x_146 = obj.numbers[3];
    const float x_149 = color.z;
    color.z = (x_149 + float(x_146));
  }
  const int x_153 = obj.numbers[4];
  const float x_156 = color.y;
  color.y = (x_156 + float(x_153));
  const float x_160 = uv.y;
  if ((x_160 > 0.25f)) {
    const int x_165 = obj.numbers[5];
    const float x_168 = color.x;
    color.x = (x_168 + float(x_165));
  }
  const float x_172 = uv.y;
  if ((x_172 > 0.5f)) {
    const int x_177 = obj.numbers[6];
    const float x_180 = color.y;
    color.y = (x_180 + float(x_177));
  }
  const float x_184 = uv.y;
  if ((x_184 > 0.75f)) {
    const int x_189 = obj.numbers[7];
    const float x_192 = color.z;
    color.z = (x_192 + float(x_189));
  }
  const int x_196 = obj.numbers[8];
  const float x_199 = color.z;
  color.z = (x_199 + float(x_196));
  const float x_203 = uv.x;
  const float x_205 = uv.y;
  if ((abs((x_203 - x_205)) < 0.25f)) {
    const int x_212 = obj.numbers[9];
    const float x_215 = color.x;
    color.x = (x_215 + float(x_212));
  }
  const float3 x_219 = normalize(color);
  x_GLF_color = float4(x_219.x, x_219.y, x_219.z, 1.0f);
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
C:\src\tint\test\Shader@0x000001609F0E92A0(128,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x000001609F0E92A0(127,12-45): error X3531: can't unroll loops marked with loop attribute

