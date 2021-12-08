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

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_239 = i;
  const int x_241 = obj.numbers[x_239];
  temp = x_241;
  const int x_242 = i;
  const int x_243 = j;
  const int x_245 = obj.numbers[x_243];
  obj.numbers[x_242] = x_245;
  const int x_247 = j;
  obj.numbers[x_247] = temp;
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
  const int x_251 = h;
  const int x_253 = obj.numbers[x_251];
  pivot = x_253;
  const int x_254 = l;
  i_1 = (x_254 - 1);
  const int x_256 = l;
  j_1 = x_256;
  [loop] while (true) {
    const int x_261 = j_1;
    const int x_262 = h;
    if ((x_261 <= (x_262 - 1))) {
    } else {
      break;
    }
    const int x_268 = obj.numbers[j_1];
    if ((x_268 <= pivot)) {
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
  const int x_282 = h;
  param_3 = x_282;
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
  const int x_288 = (top + 1);
  top = x_288;
  stack[x_288] = l_1;
  const int x_292 = (top + 1);
  top = x_292;
  stack[x_292] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_302 = top;
    top = (x_302 - 1);
    const int x_305 = stack[x_302];
    h_1 = x_305;
    const int x_306 = top;
    top = (x_306 - 1);
    const int x_309 = stack[x_306];
    l_1 = x_309;
    param_4 = l_1;
    param_5 = h_1;
    const int x_312 = performPartition_i1_i1_(param_4, param_5);
    p = x_312;
    if (((p - 1) > l_1)) {
      const int x_320 = (top + 1);
      top = x_320;
      stack[x_320] = l_1;
      const int x_324 = (top + 1);
      top = x_324;
      stack[x_324] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_335 = (top + 1);
      top = x_335;
      stack[x_335] = (p + 1);
      const int x_340 = (top + 1);
      top = x_340;
      stack[x_340] = h_1;
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
      const int x_104 = i_2;
      const int x_107 = obj.numbers[i_2];
      const int x_110 = obj.numbers[i_2];
      obj.numbers[x_104] = (x_107 * x_110);
    }
  }
  quicksort_();
  const float4 x_116 = x_GLF_FragCoord;
  const float2 x_119 = asfloat(x_34[0].xy);
  uv = (float2(x_116.x, x_116.y) / x_119);
  color = float3(1.0f, 2.0f, 3.0f);
  const int x_122 = obj.numbers[0];
  const float x_125 = color.x;
  color.x = (x_125 + float(x_122));
  const float x_129 = uv.x;
  if ((x_129 > 0.25f)) {
    const int x_134 = obj.numbers[1];
    const float x_137 = color.x;
    color.x = (x_137 + float(x_134));
  }
  const float x_141 = uv.x;
  if ((x_141 > 0.5f)) {
    const int x_146 = obj.numbers[2];
    const float x_149 = color.y;
    color.y = (x_149 + float(x_146));
  }
  const float x_153 = uv.x;
  if ((x_153 > 0.75f)) {
    const int x_158 = obj.numbers[3];
    const float x_161 = color.z;
    color.z = (x_161 + float(x_158));
  }
  const int x_165 = obj.numbers[4];
  const float x_168 = color.y;
  color.y = (x_168 + float(x_165));
  const float x_172 = uv.y;
  if ((x_172 > 0.25f)) {
    const int x_177 = obj.numbers[5];
    const float x_180 = color.x;
    color.x = (x_180 + float(x_177));
  }
  const float x_184 = uv.y;
  if ((x_184 > 0.5f)) {
    const int x_189 = obj.numbers[6];
    const float x_192 = color.y;
    color.y = (x_192 + float(x_189));
  }
  const float x_196 = uv.y;
  if ((x_196 > 0.75f)) {
    const int x_201 = obj.numbers[7];
    const float x_204 = color.z;
    color.z = (x_204 + float(x_201));
  }
  const int x_208 = obj.numbers[8];
  const float x_211 = color.z;
  color.z = (x_211 + float(x_208));
  const float x_215 = uv.x;
  const float x_217 = uv.y;
  if ((abs((x_215 - x_217)) < 0.25f)) {
    const int x_224 = obj.numbers[9];
    const float x_227 = color.x;
    color.x = (x_227 + float(x_224));
  }
  const float3 x_231 = normalize(color);
  frag_color = float4(x_231.x, x_231.y, x_231.z, 1.0f);
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
C:\src\tint\test\Shader@0x000001E481EDD080(130,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x000001E481EDD080(129,12-45): error X3531: can't unroll loops marked with loop attribute

