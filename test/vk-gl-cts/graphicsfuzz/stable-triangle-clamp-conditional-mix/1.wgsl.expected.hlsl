cbuffer cbuffer_x_15 : register(b0, space0) {
  uint4 x_15[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float cross2d_vf2_vf2_(inout float2 a, inout float2 b) {
  const float x_85 = a.x;
  const float x_87 = b.y;
  const float x_90 = b.x;
  const float x_92 = a.y;
  return ((x_85 * x_87) - (x_90 * x_92));
}

int pointInTriangle_vf2_vf2_vf2_vf2_(inout float2 p, inout float2 a_1, inout float2 b_1, inout float2 c) {
  float var_y = 0.0f;
  float x_96 = 0.0f;
  float x_97 = 0.0f;
  float clamp_y = 0.0f;
  float pab = 0.0f;
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  float pbc = 0.0f;
  float2 param_2 = float2(0.0f, 0.0f);
  float2 param_3 = float2(0.0f, 0.0f);
  float pca = 0.0f;
  float2 param_4 = float2(0.0f, 0.0f);
  float2 param_5 = float2(0.0f, 0.0f);
  bool x_173 = false;
  bool x_205 = false;
  bool x_174_phi = false;
  bool x_206_phi = false;
  const float x_99 = asfloat(x_15[0].x);
  const float x_101 = asfloat(x_15[0].y);
  if ((x_99 == x_101)) {
    const float x_107 = c.y;
    const float2 x_108 = float2(0.0f, x_107);
    if (true) {
      const float x_112 = c.y;
      x_97 = x_112;
    } else {
      x_97 = 1.0f;
    }
    const float x_113 = x_97;
    const float x_114 = c.y;
    const float2 x_116 = float2(1.0f, max(x_113, x_114));
    const float2 x_117 = float2(x_108.x, x_108.y);
    x_96 = x_107;
  } else {
    x_96 = -1.0f;
  }
  var_y = x_96;
  const float x_120 = c.y;
  const float x_121 = c.y;
  clamp_y = clamp(x_120, x_121, var_y);
  const float x_125 = p.x;
  const float x_127 = a_1.x;
  const float x_130 = p.y;
  const float x_132 = a_1.y;
  const float x_136 = b_1.x;
  const float x_137 = a_1.x;
  const float x_140 = b_1.y;
  const float x_141 = a_1.y;
  param = float2((x_125 - x_127), (x_130 - x_132));
  param_1 = float2((x_136 - x_137), (x_140 - x_141));
  const float x_144 = cross2d_vf2_vf2_(param, param_1);
  pab = x_144;
  const float x_145 = p.x;
  const float x_146 = b_1.x;
  const float x_148 = p.y;
  const float x_149 = b_1.y;
  const float x_153 = c.x;
  const float x_154 = b_1.x;
  const float x_156 = clamp_y;
  const float x_157 = b_1.y;
  param_2 = float2((x_145 - x_146), (x_148 - x_149));
  param_3 = float2((x_153 - x_154), (x_156 - x_157));
  const float x_160 = cross2d_vf2_vf2_(param_2, param_3);
  pbc = x_160;
  bool tint_tmp = (pab < 0.0f);
  if (tint_tmp) {
    tint_tmp = (pbc < 0.0f);
  }
  const bool x_165 = (tint_tmp);
  x_174_phi = x_165;
  if (!(x_165)) {
    bool tint_tmp_1 = (pab >= 0.0f);
    if (tint_tmp_1) {
      tint_tmp_1 = (pbc >= 0.0f);
    }
    x_173 = (tint_tmp_1);
    x_174_phi = x_173;
  }
  if (!(x_174_phi)) {
    return 0;
  }
  const float x_178 = p.x;
  const float x_179 = c.x;
  const float x_181 = p.y;
  const float x_182 = c.y;
  const float x_185 = a_1.x;
  const float x_186 = c.x;
  const float x_188 = a_1.y;
  const float x_189 = c.y;
  param_4 = float2((x_178 - x_179), (x_181 - x_182));
  param_5 = float2((x_185 - x_186), (x_188 - x_189));
  const float x_192 = cross2d_vf2_vf2_(param_4, param_5);
  pca = x_192;
  bool tint_tmp_2 = (pab < 0.0f);
  if (tint_tmp_2) {
    tint_tmp_2 = (pca < 0.0f);
  }
  const bool x_197 = (tint_tmp_2);
  x_206_phi = x_197;
  if (!(x_197)) {
    bool tint_tmp_3 = (pab >= 0.0f);
    if (tint_tmp_3) {
      tint_tmp_3 = (pca >= 0.0f);
    }
    x_205 = (tint_tmp_3);
    x_206_phi = x_205;
  }
  if (!(x_206_phi)) {
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
  const float4 x_72 = gl_FragCoord;
  const float2 x_75 = asfloat(x_15[0].xy);
  pos = (float2(x_72.x, x_72.y) / x_75);
  param_6 = pos;
  param_7 = float2(0.699999988f, 0.300000012f);
  param_8 = float2(0.5f, 0.899999976f);
  param_9 = float2(0.100000001f, 0.400000006f);
  const int x_78 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_78 == 1)) {
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
