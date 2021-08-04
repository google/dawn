static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_17 : register(b0, space0) {
  uint4 x_17[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int pointInTriangle_vf2_vf2_vf2_vf2_(inout float2 p, inout float2 a, inout float2 b, inout float2 c) {
  float x_66 = 0.0f;
  float x_67 = 0.0f;
  float x_68 = 0.0f;
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  float2 param_2 = float2(0.0f, 0.0f);
  float2 param_3 = float2(0.0f, 0.0f);
  float2 param_4 = float2(0.0f, 0.0f);
  float2 param_5 = float2(0.0f, 0.0f);
  bool x_135 = false;
  bool x_172 = false;
  bool x_136_phi = false;
  bool x_173_phi = false;
  const float x_70 = p.x;
  const float x_72 = a.x;
  const float x_75 = p.y;
  const float x_77 = a.y;
  const float x_81 = b.x;
  const float x_82 = a.x;
  const float x_85 = b.y;
  const float x_86 = a.y;
  param = float2((x_70 - x_72), (x_75 - x_77));
  param_1 = float2((x_81 - x_82), (x_85 - x_86));
  const float x_90 = param.x;
  const float x_92 = param_1.y;
  const float x_95 = param_1.x;
  const float x_97 = param.y;
  const float x_99 = ((x_90 * x_92) - (x_95 * x_97));
  x_68 = x_99;
  const float x_100 = p.x;
  const float x_101 = b.x;
  const float x_103 = p.y;
  const float x_104 = b.y;
  const float x_108 = c.x;
  const float x_109 = b.x;
  const float x_112 = c.y;
  const float x_113 = b.y;
  param_2 = float2((x_100 - x_101), (x_103 - x_104));
  param_3 = float2((x_108 - x_109), (x_112 - x_113));
  const float x_117 = param_2.x;
  const float x_119 = param_3.y;
  const float x_122 = param_3.x;
  const float x_124 = param_2.y;
  const float x_126 = ((x_117 * x_119) - (x_122 * x_124));
  x_67 = x_126;
  const bool x_127 = (x_99 < 0.0f);
  bool tint_tmp = x_127;
  if (tint_tmp) {
    tint_tmp = (x_126 < 0.0f);
  }
  const bool x_129 = (tint_tmp);
  x_136_phi = x_129;
  if (!(x_129)) {
    bool tint_tmp_1 = (x_99 >= 0.0f);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_126 >= 0.0f);
    }
    x_135 = (tint_tmp_1);
    x_136_phi = x_135;
  }
  if (!(x_136_phi)) {
    return 0;
  }
  const float x_140 = p.x;
  const float x_141 = c.x;
  const float x_143 = p.y;
  const float x_144 = c.y;
  const float x_147 = a.x;
  const float x_148 = c.x;
  const float x_150 = a.y;
  const float x_151 = c.y;
  param_4 = float2((x_140 - x_141), (x_143 - x_144));
  param_5 = float2((x_147 - x_148), (x_150 - x_151));
  const float x_155 = param_4.x;
  const float x_157 = param_5.y;
  const float x_160 = param_5.x;
  const float x_162 = param_4.y;
  const float x_164 = ((x_155 * x_157) - (x_160 * x_162));
  x_66 = x_164;
  bool tint_tmp_2 = x_127;
  if (tint_tmp_2) {
    tint_tmp_2 = (x_164 < 0.0f);
  }
  const bool x_166 = (tint_tmp_2);
  x_173_phi = x_166;
  if (!(x_166)) {
    bool tint_tmp_3 = (x_99 >= 0.0f);
    if (tint_tmp_3) {
      tint_tmp_3 = (x_164 >= 0.0f);
    }
    x_172 = (tint_tmp_3);
    x_173_phi = x_172;
  }
  if (!(x_173_phi)) {
    return 0;
  }
  return 1;
}

void main_1() {
  float2 param_6 = float2(0.0f, 0.0f);
  float2 param_7 = float2(0.0f, 0.0f);
  float2 param_8 = float2(0.0f, 0.0f);
  float2 param_9 = float2(0.0f, 0.0f);
  const float4 x_55 = gl_FragCoord;
  const float2 x_58 = asfloat(x_17[0].xy);
  param_6 = (float2(x_55.x, x_55.y) / x_58);
  param_7 = float2(0.699999988f, 0.300000012f);
  param_8 = float2(0.5f, 0.899999976f);
  param_9 = float2(0.100000001f, 0.400000006f);
  const int x_60 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_60 == 1)) {
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
