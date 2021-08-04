static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_24 : register(b0, space0) {
  uint4 x_24[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float cross2d_vf2_vf2_(inout float2 a, inout float2 b) {
  const float x_76 = a.x;
  const float x_78 = b.y;
  const float x_81 = b.x;
  const float x_83 = a.y;
  return ((x_76 * x_78) - (x_81 * x_83));
}

int pointInTriangle_vf2_vf2_vf2_vf2_(inout float2 p, inout float2 a_1, inout float2 b_1, inout float2 c) {
  float pab = 0.0f;
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  float pbc = 0.0f;
  float2 param_2 = float2(0.0f, 0.0f);
  float2 param_3 = float2(0.0f, 0.0f);
  float pca = 0.0f;
  float2 param_4 = float2(0.0f, 0.0f);
  float2 param_5 = float2(0.0f, 0.0f);
  bool x_145 = false;
  bool x_185 = false;
  bool x_146_phi = false;
  bool x_186_phi = false;
  const float x_88 = p.x;
  const float x_90 = a_1.x;
  const float x_93 = p.y;
  const float x_95 = a_1.y;
  const float x_99 = b_1.x;
  const float x_101 = a_1.x;
  const float x_104 = b_1.y;
  const float x_106 = a_1.y;
  param = float2((x_88 - x_90), (x_93 - x_95));
  param_1 = float2((x_99 - x_101), (x_104 - x_106));
  const float x_109 = cross2d_vf2_vf2_(param, param_1);
  pab = x_109;
  const float x_111 = p.x;
  const float x_113 = b_1.x;
  const float x_116 = p.y;
  const float x_118 = b_1.y;
  const float x_122 = c.x;
  const float x_124 = b_1.x;
  const float x_127 = c.y;
  const float x_129 = b_1.y;
  param_2 = float2((x_111 - x_113), (x_116 - x_118));
  param_3 = float2((x_122 - x_124), (x_127 - x_129));
  const float x_132 = cross2d_vf2_vf2_(param_2, param_3);
  pbc = x_132;
  const bool x_137 = ((pab < 0.0f) & (pbc < 0.0f));
  x_146_phi = x_137;
  if (!(x_137)) {
    x_145 = ((pab >= 0.0f) & (pbc >= 0.0f));
    x_146_phi = x_145;
  }
  if (!(x_146_phi)) {
    return 0;
  }
  const float x_151 = p.x;
  const float x_153 = c.x;
  const float x_156 = p.y;
  const float x_158 = c.y;
  const float x_162 = a_1.x;
  const float x_164 = c.x;
  const float x_167 = a_1.y;
  const float x_169 = c.y;
  param_4 = float2((x_151 - x_153), (x_156 - x_158));
  param_5 = float2((x_162 - x_164), (x_167 - x_169));
  const float x_172 = cross2d_vf2_vf2_(param_4, param_5);
  pca = x_172;
  const bool x_177 = ((pab < 0.0f) & (pca < 0.0f));
  x_186_phi = x_177;
  if (!(x_177)) {
    x_185 = ((pab >= 0.0f) & (pca >= 0.0f));
    x_186_phi = x_185;
  }
  if (!(x_186_phi)) {
    return 0;
  }
  return 1;
}

void main_1() {
  float2 pos = float2(0.0f, 0.0f);
  float2 param_6 = float2(0.0f, 0.0f);
  float2 param_7 = float2(0.0f, 0.0f);
  float2 param_8 = float2(0.0f, 0.0f);
  float2 param_9 = float2(0.0f, 0.0f);
  const float4 x_63 = gl_FragCoord;
  const float2 x_66 = asfloat(x_24[0].xy);
  pos = (float2(x_63.x, x_63.y) / x_66);
  param_6 = pos;
  param_7 = float2(0.699999988f, 0.300000012f);
  param_8 = float2(0.5f, 0.899999976f);
  param_9 = float2(0.100000001f, 0.400000006f);
  const int x_69 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_69 == 1)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  }
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
