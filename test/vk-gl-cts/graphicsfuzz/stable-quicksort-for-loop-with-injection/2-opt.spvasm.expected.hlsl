SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 x_GLF_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_33 : register(b0, space0) {
  uint4 x_33[1];
};
cbuffer cbuffer_x_36 : register(b1, space0) {
  uint4 x_36[1];
};
static float4 frag_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_250 = i;
  const int x_252 = obj.numbers[x_250];
  temp = x_252;
  const int x_253 = i;
  const int x_254 = j;
  const int x_256 = obj.numbers[x_254];
  obj.numbers[x_253] = x_256;
  const int x_258 = j;
  obj.numbers[x_258] = temp;
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
  const int x_262 = h;
  const int x_264 = obj.numbers[x_262];
  pivot = x_264;
  const int x_265 = l;
  i_1 = (x_265 - 1);
  const int x_267 = l;
  j_1 = x_267;
  [loop] while (true) {
    const int x_272 = j_1;
    const int x_273 = h;
    if ((x_272 <= (x_273 - 1))) {
    } else {
      break;
    }
    const int x_279 = obj.numbers[j_1];
    if ((x_279 <= pivot)) {
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
  const int x_293 = h;
  param_3 = x_293;
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
  const int x_299 = (top + 1);
  top = x_299;
  stack[x_299] = l_1;
  const int x_303 = (top + 1);
  top = x_303;
  stack[x_303] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_313 = top;
    top = (x_313 - 1);
    const int x_316 = stack[x_313];
    h_1 = x_316;
    const int x_317 = top;
    top = (x_317 - 1);
    const int x_320 = stack[x_317];
    l_1 = x_320;
    param_4 = l_1;
    param_5 = h_1;
    const int x_323 = performPartition_i1_i1_(param_4, param_5);
    p = x_323;
    if (((p - 1) > l_1)) {
      const int x_331 = (top + 1);
      top = x_331;
      stack[x_331] = l_1;
      const int x_335 = (top + 1);
      top = x_335;
      stack[x_335] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_346 = (top + 1);
      top = x_346;
      stack[x_346] = (p + 1);
      const int x_351 = (top + 1);
      top = x_351;
      stack[x_351] = h_1;
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
      const float x_109 = asfloat(x_33[0].x);
      const float x_111 = asfloat(x_33[0].y);
      if ((x_109 > x_111)) {
        break;
      }
      const int x_115 = i_2;
      const int x_118 = obj.numbers[i_2];
      const int x_121 = obj.numbers[i_2];
      obj.numbers[x_115] = (x_118 * x_121);
    }
  }
  quicksort_();
  const float4 x_127 = x_GLF_FragCoord;
  const float2 x_130 = asfloat(x_36[0].xy);
  uv = (float2(x_127.x, x_127.y) / x_130);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_133 = obj.numbers[0];
  const float x_136 = color.x;
  color.x = (x_136 + float(x_133));
  const float x_140 = uv.x;
  if ((x_140 > 0.25f)) {
    const int x_145 = obj.numbers[1];
    const float x_148 = color.x;
    color.x = (x_148 + float(x_145));
  }
  const float x_152 = uv.x;
  if ((x_152 > 0.5f)) {
    const int x_157 = obj.numbers[2];
    const float x_160 = color.y;
    color.y = (x_160 + float(x_157));
  }
  const float x_164 = uv.x;
  if ((x_164 > 0.75f)) {
    const int x_169 = obj.numbers[3];
    const float x_172 = color.z;
    color.z = (x_172 + float(x_169));
  }
  const int x_176 = obj.numbers[4];
  const float x_179 = color.y;
  color.y = (x_179 + float(x_176));
  const float x_183 = uv.y;
  if ((x_183 > 0.25f)) {
    const int x_188 = obj.numbers[5];
    const float x_191 = color.x;
    color.x = (x_191 + float(x_188));
  }
  const float x_195 = uv.y;
  if ((x_195 > 0.5f)) {
    const int x_200 = obj.numbers[6];
    const float x_203 = color.y;
    color.y = (x_203 + float(x_200));
  }
  const float x_207 = uv.y;
  if ((x_207 > 0.75f)) {
    const int x_212 = obj.numbers[7];
    const float x_215 = color.z;
    color.z = (x_215 + float(x_212));
  }
  const int x_219 = obj.numbers[8];
  const float x_222 = color.z;
  color.z = (x_222 + float(x_219));
  const float x_226 = uv.x;
  const float x_228 = uv.y;
  if ((abs((x_226 - x_228)) < 0.25f)) {
    const int x_235 = obj.numbers[9];
    const float x_238 = color.x;
    color.x = (x_238 + float(x_235));
  }
  const float3 x_242 = normalize(color);
  frag_color = float4(x_242.x, x_242.y, x_242.z, 1.0f);
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
C:\src\tint\test\Shader@0x00000277FFA71560(133,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x00000277FFA71560(132,12-45): error X3531: can't unroll loops marked with loop attribute

