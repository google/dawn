static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_20 : register(b0, space0) {
  uint4 x_20[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int index = 0;
static int state[16] = (int[16])0;

bool collision_vf2_vf4_(inout float2 pos, inout float4 quad) {
  bool4 x_116 = bool4(false, false, false, false);
  const float x_118 = pos.x;
  const float x_120 = quad.x;
  if ((x_118 < x_120)) {
    return false;
  }
  const float x_125 = pos.y;
  const float x_127 = quad.y;
  if ((x_125 < x_127)) {
    return false;
  }
  const float x_132 = pos.x;
  const float x_134 = quad.x;
  const float x_136 = quad.z;
  if ((x_132 > (x_134 + x_136))) {
    return false;
  }
  const float x_142 = pos.y;
  const float x_144 = quad.y;
  const float x_146 = quad.w;
  if ((x_142 > (x_144 + x_146))) {
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
      const int x_159 = i;
      const float2 x_160 = pos_1;
      param = x_160;
      const float4 tint_symbol_4[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      indexable = tint_symbol_4;
      const float4 x_162 = indexable[x_159];
      param_1 = x_162;
      const bool x_163 = collision_vf2_vf4_(param, param_1);
      if (x_163) {
        const int x_166 = i;
        const float4 tint_symbol_5[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_1 = tint_symbol_5;
        const float x_168 = indexable_1[x_166].x;
        const int x_170 = i;
        const float4 tint_symbol_6[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_2 = tint_symbol_6;
        const float x_172 = indexable_2[x_170].y;
        const int x_175 = i;
        const float4 tint_symbol_7[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
        indexable_3 = tint_symbol_7;
        const float4 x_181 = indexable_3[((((int(x_168) * int(x_172)) + (x_175 * 9)) + 11) % 16)];
        res = x_181;
      }
    }
  }
  return res;
}

void main_1() {
  float2 lin = float2(0.0f, 0.0f);
  float2 param_2 = float2(0.0f, 0.0f);
  const float4 x_105 = gl_FragCoord;
  const float2 x_108 = asfloat(x_20[0].xy);
  lin = (float2(x_105.x, x_105.y) / x_108);
  lin = floor((lin * 32.0f));
  param_2 = lin;
  const float4 x_114 = match_vf2_(param_2);
  x_GLF_color = x_114;
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
