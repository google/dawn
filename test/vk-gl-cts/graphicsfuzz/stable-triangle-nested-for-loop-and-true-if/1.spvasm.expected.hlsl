cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_19 : register(b0, space0) {
  uint4 x_19[1];
};

int pointInTriangle_vf2_vf2_vf2_vf2_(inout float2 p, inout float2 a, inout float2 b, inout float2 c) {
  float x_78 = 0.0f;
  float x_79 = 0.0f;
  float x_80 = 0.0f;
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  float2 param_2 = float2(0.0f, 0.0f);
  float2 param_3 = float2(0.0f, 0.0f);
  float2 param_4 = float2(0.0f, 0.0f);
  float2 param_5 = float2(0.0f, 0.0f);
  bool x_147 = false;
  bool x_203 = false;
  bool x_148_phi = false;
  bool x_204_phi = false;
  const float x_82 = p.x;
  const float x_84 = a.x;
  const float x_87 = p.y;
  const float x_89 = a.y;
  const float x_93 = b.x;
  const float x_94 = a.x;
  const float x_97 = b.y;
  const float x_98 = a.y;
  param = float2((x_82 - x_84), (x_87 - x_89));
  param_1 = float2((x_93 - x_94), (x_97 - x_98));
  const float x_102 = param.x;
  const float x_104 = param_1.y;
  const float x_107 = param_1.x;
  const float x_109 = param.y;
  const float x_111 = ((x_102 * x_104) - (x_107 * x_109));
  x_80 = x_111;
  const float x_112 = p.x;
  const float x_113 = b.x;
  const float x_115 = p.y;
  const float x_116 = b.y;
  const float x_120 = c.x;
  const float x_121 = b.x;
  const float x_124 = c.y;
  const float x_125 = b.y;
  param_2 = float2((x_112 - x_113), (x_115 - x_116));
  param_3 = float2((x_120 - x_121), (x_124 - x_125));
  const float x_129 = param_2.x;
  const float x_131 = param_3.y;
  const float x_134 = param_3.x;
  const float x_136 = param_2.y;
  const float x_138 = ((x_129 * x_131) - (x_134 * x_136));
  x_79 = x_138;
  const bool x_139 = (x_111 < 0.0f);
  const bool x_141 = (x_139 & (x_138 < 0.0f));
  x_148_phi = x_141;
  if (!(x_141)) {
    x_147 = ((x_111 >= 0.0f) & (x_138 >= 0.0f));
    x_148_phi = x_147;
  }
  int x_153_phi = 0;
  if (!(x_148_phi)) {
    x_153_phi = 0;
    while (true) {
      int x_154 = 0;
      int x_164_phi = 0;
      const int x_153 = x_153_phi;
      const float x_159 = asfloat(x_11[0].y);
      const int x_160 = int(x_159);
      if ((x_153 < x_160)) {
      } else {
        break;
      }
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      x_164_phi = 0;
      while (true) {
        int x_165 = 0;
        const int x_164 = x_164_phi;
        if ((x_164 < x_160)) {
        } else {
          break;
        }
        x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
        {
          x_165 = (x_164 + 1);
          x_164_phi = x_165;
        }
      }
      {
        x_154 = (x_153 + 1);
        x_153_phi = x_154;
      }
    }
    return 0;
  }
  const float x_171 = p.x;
  const float x_172 = c.x;
  const float x_174 = p.y;
  const float x_175 = c.y;
  const float x_178 = a.x;
  const float x_179 = c.x;
  const float x_181 = a.y;
  const float x_182 = c.y;
  param_4 = float2((x_171 - x_172), (x_174 - x_175));
  param_5 = float2((x_178 - x_179), (x_181 - x_182));
  const float x_186 = param_4.x;
  const float x_188 = param_5.y;
  const float x_191 = param_5.x;
  const float x_193 = param_4.y;
  const float x_195 = ((x_186 * x_188) - (x_191 * x_193));
  x_78 = x_195;
  const bool x_197 = (x_139 & (x_195 < 0.0f));
  x_204_phi = x_197;
  if (!(x_197)) {
    x_203 = ((x_111 >= 0.0f) & (x_195 >= 0.0f));
    x_204_phi = x_203;
  }
  if (!(x_204_phi)) {
    return 0;
  }
  return 1;
}

void main_1() {
  float2 param_6 = float2(0.0f, 0.0f);
  float2 param_7 = float2(0.0f, 0.0f);
  float2 param_8 = float2(0.0f, 0.0f);
  float2 param_9 = float2(0.0f, 0.0f);
  const float4 x_60 = gl_FragCoord;
  const float2 x_63 = asfloat(x_19[0].xy);
  param_6 = (float2(x_60.x, x_60.y) / x_63);
  param_7 = float2(0.699999988f, 0.300000012f);
  param_8 = float2(0.5f, 0.899999976f);
  param_9 = float2(0.100000001f, 0.400000006f);
  const int x_65 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_65 == 1)) {
    const float x_71 = asfloat(x_11[0].y);
    const float x_73 = asfloat(x_11[0].x);
    if ((x_71 >= x_73)) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
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
