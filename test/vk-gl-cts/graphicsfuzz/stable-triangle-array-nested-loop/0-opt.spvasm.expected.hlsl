static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_24 : register(b0, space0) {
  uint4 x_24[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float cross2d_vf2_vf2_(inout float2 a, inout float2 b) {
  const float x_79 = a.x;
  const float x_81 = b.y;
  const float x_84 = b.x;
  const float x_86 = a.y;
  return ((x_79 * x_81) - (x_84 * x_86));
}

int pointInTriangle_vf2_vf2_vf2_vf2_(inout float2 p, inout float2 a_1, inout float2 b_1, inout float2 c) {
  bool x_90 = false;
  int x_91 = 0;
  float pab = 0.0f;
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  float pbc = 0.0f;
  float2 param_2 = float2(0.0f, 0.0f);
  float2 param_3 = float2(0.0f, 0.0f);
  float pca = 0.0f;
  float2 param_4 = float2(0.0f, 0.0f);
  float2 param_5 = float2(0.0f, 0.0f);
  bool x_140 = false;
  bool x_168 = false;
  bool x_141_phi = false;
  bool x_169_phi = false;
  int x_173_phi = 0;
  switch(0u) {
    default: {
      const float x_95 = p.x;
      const float x_97 = a_1.x;
      const float x_100 = p.y;
      const float x_102 = a_1.y;
      const float x_106 = b_1.x;
      const float x_107 = a_1.x;
      const float x_110 = b_1.y;
      const float x_111 = a_1.y;
      param = float2((x_95 - x_97), (x_100 - x_102));
      param_1 = float2((x_106 - x_107), (x_110 - x_111));
      const float x_114 = cross2d_vf2_vf2_(param, param_1);
      pab = x_114;
      const float x_115 = p.x;
      const float x_116 = b_1.x;
      const float x_118 = p.y;
      const float x_119 = b_1.y;
      const float x_123 = c.x;
      const float x_124 = b_1.x;
      const float x_127 = c.y;
      const float x_128 = b_1.y;
      param_2 = float2((x_115 - x_116), (x_118 - x_119));
      param_3 = float2((x_123 - x_124), (x_127 - x_128));
      const float x_131 = cross2d_vf2_vf2_(param_2, param_3);
      pbc = x_131;
      const bool x_134 = ((x_114 < 0.0f) & (x_131 < 0.0f));
      x_141_phi = x_134;
      if (!(x_134)) {
        x_140 = ((x_114 >= 0.0f) & (x_131 >= 0.0f));
        x_141_phi = x_140;
      }
      if (!(x_141_phi)) {
        x_90 = true;
        x_91 = 0;
        x_173_phi = 0;
        break;
      }
      const float x_145 = p.x;
      const float x_146 = c.x;
      const float x_148 = p.y;
      const float x_149 = c.y;
      const float x_152 = a_1.x;
      const float x_153 = c.x;
      const float x_155 = a_1.y;
      const float x_156 = c.y;
      param_4 = float2((x_145 - x_146), (x_148 - x_149));
      param_5 = float2((x_152 - x_153), (x_155 - x_156));
      const float x_159 = cross2d_vf2_vf2_(param_4, param_5);
      pca = x_159;
      const bool x_162 = ((x_114 < 0.0f) & (x_159 < 0.0f));
      x_169_phi = x_162;
      if (!(x_162)) {
        x_168 = ((x_114 >= 0.0f) & (x_159 >= 0.0f));
        x_169_phi = x_168;
      }
      if (!(x_169_phi)) {
        x_90 = true;
        x_91 = 0;
        x_173_phi = 0;
        break;
      }
      x_90 = true;
      x_91 = 1;
      x_173_phi = 1;
      break;
    }
  }
  return x_173_phi;
}

void main_1() {
  float2 pos = float2(0.0f, 0.0f);
  float2 param_6 = float2(0.0f, 0.0f);
  float2 param_7 = float2(0.0f, 0.0f);
  float2 param_8 = float2(0.0f, 0.0f);
  float2 param_9 = float2(0.0f, 0.0f);
  const float4 x_67 = gl_FragCoord;
  const float2 x_70 = asfloat(x_24[0].xy);
  const float2 x_71 = (float2(x_67.x, x_67.y) / x_70);
  pos = x_71;
  param_6 = x_71;
  param_7 = float2(0.699999988f, 0.300000012f);
  param_8 = float2(0.5f, 0.899999976f);
  param_9 = float2(0.100000001f, 0.400000006f);
  const int x_72 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_72 == 1)) {
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
