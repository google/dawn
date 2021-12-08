SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 x_GLF_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_34 : register(b1, space0) {
  uint4 x_34[1];
};
cbuffer cbuffer_x_37 : register(b0, space0) {
  uint4 x_37[1];
};
static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_257 = i;
  const int x_259 = obj.numbers[x_257];
  temp = x_259;
  const int x_260 = i;
  const int x_261 = j;
  const int x_263 = obj.numbers[x_261];
  obj.numbers[x_260] = x_263;
  const int x_265 = j;
  obj.numbers[x_265] = temp;
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
  const int x_269 = h;
  const int x_271 = obj.numbers[x_269];
  pivot = x_271;
  const int x_272 = l;
  i_1 = (x_272 - 1);
  const int x_274 = l;
  j_1 = x_274;
  [loop] while (true) {
    const int x_279 = j_1;
    const int x_280 = h;
    if ((x_279 <= (x_280 - 1))) {
    } else {
      break;
    }
    const int x_286 = obj.numbers[j_1];
    if ((x_286 <= pivot)) {
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
  const int x_300 = h;
  param_3 = x_300;
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
  const int x_306 = (top + 1);
  top = x_306;
  stack[x_306] = l_1;
  const int x_310 = (top + 1);
  top = x_310;
  stack[x_310] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_320 = top;
    top = (x_320 - 1);
    const int x_323 = stack[x_320];
    h_1 = x_323;
    const int x_324 = top;
    top = (x_324 - 1);
    const int x_327 = stack[x_324];
    l_1 = x_327;
    param_4 = l_1;
    param_5 = h_1;
    const int x_330 = performPartition_i1_i1_(param_4, param_5);
    p = x_330;
    if (((p - 1) > l_1)) {
      const int x_338 = (top + 1);
      top = x_338;
      stack[x_338] = l_1;
      const int x_342 = (top + 1);
      top = x_342;
      stack[x_342] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_353 = (top + 1);
      top = x_353;
      stack[x_353] = (p + 1);
      const int x_358 = (top + 1);
      top = x_358;
      stack[x_358] = h_1;
    }
  }
  return;
}

void main_1() {
  int i_2 = 0;
  float2 uv = float2(0.0f, 0.0f);
  float3 color = float3(0.0f, 0.0f, 0.0f);
  x_GLF_FragCoord = ((x_GLF_pos + float4(1.0f, 1.0f, 0.0f, 0.0f)) * float4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    [loop] for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      obj.numbers[i_2] = (10 - i_2);
      const int x_108 = i_2;
      const int x_111 = obj.numbers[i_2];
      const int x_114 = obj.numbers[i_2];
      obj.numbers[x_108] = (x_111 * x_114);
    }
  }
  quicksort_();
  const float4 x_120 = x_GLF_FragCoord;
  const float2 x_123 = asfloat(x_34[0].xy);
  uv = (float2(x_120.x, x_120.y) / x_123);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_126 = obj.numbers[0];
  const float x_129 = color.x;
  color.x = (x_129 + float(x_126));
  const float x_133 = uv.x;
  if ((x_133 > 0.25f)) {
    const int x_138 = obj.numbers[1];
    const float x_141 = color.x;
    color.x = (x_141 + float(x_138));
  }
  const float x_145 = uv.x;
  if ((x_145 > 0.5f)) {
    const float x_150 = asfloat(x_37[0].y);
    const int x_155 = obj.numbers[max((2 * int(x_150)), 2)];
    const float x_158 = asfloat(x_37[0].y);
    const int x_163 = obj.numbers[max((2 * int(x_158)), 2)];
    const float x_167 = color.y;
    color.y = (x_167 + max(float(x_155), float(x_163)));
  }
  const float x_171 = uv.x;
  if ((x_171 > 0.75f)) {
    const int x_176 = obj.numbers[3];
    const float x_179 = color.z;
    color.z = (x_179 + float(x_176));
  }
  const int x_183 = obj.numbers[4];
  const float x_186 = color.y;
  color.y = (x_186 + float(x_183));
  const float x_190 = uv.y;
  if ((x_190 > 0.25f)) {
    const int x_195 = obj.numbers[5];
    const float x_198 = color.x;
    color.x = (x_198 + float(x_195));
  }
  const float x_202 = uv.y;
  if ((x_202 > 0.5f)) {
    const int x_207 = obj.numbers[6];
    const float x_210 = color.y;
    color.y = (x_210 + float(x_207));
  }
  const float x_214 = uv.y;
  if ((x_214 > 0.75f)) {
    const int x_219 = obj.numbers[7];
    const float x_222 = color.z;
    color.z = (x_222 + float(x_219));
  }
  const int x_226 = obj.numbers[8];
  const float x_229 = color.z;
  color.z = (x_229 + float(x_226));
  const float x_233 = uv.x;
  const float x_235 = uv.y;
  if ((abs((x_233 - x_235)) < 0.25f)) {
    const int x_242 = obj.numbers[9];
    const float x_245 = color.x;
    color.x = (x_245 + float(x_242));
  }
  const float3 x_249 = normalize(color);
  frag_color = float4(x_249.x, x_249.y, x_249.z, 1.0f);
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
  const main_out tint_symbol_5 = {frag_color, gl_Position};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_GLF_pos_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.frag_color_1 = inner_result.frag_color_1;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000015776479900(133,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x0000015776479900(132,12-45): error X3531: can't unroll loops marked with loop attribute

