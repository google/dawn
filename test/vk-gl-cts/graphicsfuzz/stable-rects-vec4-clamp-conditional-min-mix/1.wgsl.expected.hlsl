static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_20 : register(b0, space0) {
  uint4 x_20[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

bool collision_vf2_vf4_(inout float2 pos, inout float4 quad) {
  const float x_110 = pos.x;
  const float x_112 = quad.x;
  if ((x_110 < x_112)) {
    return false;
  }
  const float x_117 = pos.y;
  const float x_119 = quad.y;
  if ((x_117 < x_119)) {
    return false;
  }
  const float x_124 = pos.x;
  const float x_126 = quad.x;
  const float x_128 = quad.z;
  if ((x_124 > (x_126 + x_128))) {
    return false;
  }
  const float x_134 = pos.y;
  const float x_136 = quad.y;
  const float x_138 = quad.w;
  if ((x_134 > (x_136 + x_138))) {
    return false;
  }
  return true;
}

float4 match_vf2_(inout float2 pos_1) {
  float4 res = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_144 = 0.0f;
  float x_145 = 0.0f;
  int i = 0;
  float2 param = float2(0.0f, 0.0f);
  float4 param_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 indexable[8] = (float4[8])0;
  float4 indexable_1[8] = (float4[8])0;
  float4 indexable_2[8] = (float4[8])0;
  float4 indexable_3[16] = (float4[16])0;
  const float x_147 = gl_FragCoord.x;
  if ((x_147 < 0.0f)) {
    x_144 = -1.0f;
  } else {
    const float x_153 = gl_FragCoord.x;
    if ((x_153 >= 0.0f)) {
      const float x_159 = gl_FragCoord.x;
      x_145 = ((x_159 >= 0.0f) ? 0.5f : 1.0f);
    } else {
      x_145 = 1.0f;
    }
    x_144 = min(x_145, 0.5f);
  }
  res = float4(clamp(0.5f, 0.5f, x_144), 0.5f, 1.0f, 1.0f);
  i = 0;
  {
    for(; (i < 8); i = (i + 1)) {
      const int x_174 = i;
      const float2 x_175 = pos_1;
      param = x_175;
      const float4 tint_symbol_4[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      indexable = tint_symbol_4;
      const float4 x_177 = indexable[x_174];
      param_1 = x_177;
      const bool x_178 = collision_vf2_vf4_(param, param_1);
      if (x_178) {
        const int x_181 = i;
        const float4 tint_symbol_5[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_1 = tint_symbol_5;
        const float x_183 = indexable_1[x_181].x;
        const int x_185 = i;
        const float4 tint_symbol_6[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
        indexable_2 = tint_symbol_6;
        const float x_187 = indexable_2[x_185].y;
        const int x_190 = i;
        const float4 tint_symbol_7[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
        indexable_3 = tint_symbol_7;
        const float4 x_196 = indexable_3[((((int(x_183) * int(x_187)) + (x_190 * 9)) + 11) % 16)];
        res = x_196;
      }
    }
  }
  return res;
}

void main_1() {
  float2 lin = float2(0.0f, 0.0f);
  float2 param_2 = float2(0.0f, 0.0f);
  const float4 x_98 = gl_FragCoord;
  const float2 x_101 = asfloat(x_20[0].xy);
  lin = (float2(x_98.x, x_98.y) / x_101);
  lin = floor((lin * 32.0f));
  param_2 = lin;
  const float4 x_107 = match_vf2_(param_2);
  x_GLF_color = x_107;
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
