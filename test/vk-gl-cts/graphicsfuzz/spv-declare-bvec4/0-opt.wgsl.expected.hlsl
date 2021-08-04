static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_20 : register(b0, space0) {
  uint4 x_20[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int index = 0;
static int state[16] = (int[16])0;

bool collision_vf2_vf4_(inout float2 pos, inout float4 quad) {
  const float x_114 = pos.x;
  const float x_116 = quad.x;
  if ((x_114 < x_116)) {
    return false;
  }
  const float x_121 = pos.y;
  const float x_123 = quad.y;
  if ((x_121 < x_123)) {
    return false;
  }
  const float x_128 = pos.x;
  const float x_130 = quad.x;
  const float x_132 = quad.z;
  if ((x_128 > (x_130 + x_132))) {
    return false;
  }
  const float x_138 = pos.y;
  const float x_140 = quad.y;
  const float x_142 = quad.w;
  if ((x_138 > (x_140 + x_142))) {
    return false;
  }
  return true;
}

float4 match_vf2_(inout float2 pos_1) {
  float4 res = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  float2 param = float2(0.0f, 0.0f);
  float4 param_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 indexable[8] = (float4[8])0;
  float4 indexable_1[8] = (float4[8])0;
  float4 indexable_2[8] = (float4[8])0;
  float4 indexable_3[16] = (float4[16])0;
  res = float4(0.5f, 0.5f, 1.0f, 1.0f);
  i = 0;
  {
    for(; (i < 8); i = (i + 1)) {
      const int x_155 = i;
      const float2 x_156 = pos_1;
      param = x_156;
      const float4 tint_symbol_4[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      indexable = tint_symbol_4;
      const float4 x_158 = indexable[x_155];
      param_1 = x_158;
      const bool x_159 = collision_vf2_vf4_(param, param_1);
      if (x_159) {
        const int x_162 = i;
        const float4 tint_symbol_5[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_1 = tint_symbol_5;
        const float x_164 = indexable_1[x_162].x;
        const int x_166 = i;
        const float4 tint_symbol_6[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_2 = tint_symbol_6;
        const float x_168 = indexable_2[x_166].y;
        const int x_171 = i;
        const float4 tint_symbol_7[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
        indexable_3 = tint_symbol_7;
        const float4 x_177 = indexable_3[((((int(x_164) * int(x_168)) + (x_171 * 9)) + 11) % 16)];
        res = x_177;
      }
    }
  }
  return res;
}

void main_1() {
  float2 lin = float2(0.0f, 0.0f);
  float2 param_2 = float2(0.0f, 0.0f);
  const float4 x_102 = gl_FragCoord;
  const float2 x_105 = asfloat(x_20[0].xy);
  lin = (float2(x_102.x, x_102.y) / x_105);
  lin = floor((lin * 32.0f));
  param_2 = lin;
  const float4 x_111 = match_vf2_(param_2);
  x_GLF_color = x_111;
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
  const main_out tint_symbol_8 = {x_GLF_color};
  return tint_symbol_8;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
