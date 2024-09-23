SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a, inout vec2 b, inout vec2 c) {
  float x_66 = 0.0f;
  float x_67 = 0.0f;
  float x_68 = 0.0f;
  vec2 param = vec2(0.0f);
  vec2 param_1 = vec2(0.0f);
  vec2 param_2 = vec2(0.0f);
  vec2 param_3 = vec2(0.0f);
  vec2 param_4 = vec2(0.0f);
  vec2 param_5 = vec2(0.0f);
  bool x_135 = false;
  bool x_172 = false;
  bool x_136_phi = false;
  bool x_173_phi = false;
  float x_70 = p.x;
  float x_72 = a.x;
  float x_75 = p.y;
  float x_77 = a.y;
  float x_81 = b.x;
  float x_82 = a.x;
  float x_85 = b.y;
  float x_86 = a.y;
  param = vec2((x_70 - x_72), (x_75 - x_77));
  param_1 = vec2((x_81 - x_82), (x_85 - x_86));
  float x_90 = param.x;
  float x_92 = param_1.y;
  float x_95 = param_1.x;
  float x_97 = param.y;
  float x_99 = ((x_90 * x_92) - (x_95 * x_97));
  x_68 = x_99;
  float x_100 = p.x;
  float x_101 = b.x;
  float x_103 = p.y;
  float x_104 = b.y;
  float x_108 = c.x;
  float x_109 = b.x;
  float x_112 = c.y;
  float x_113 = b.y;
  param_2 = vec2((x_100 - x_101), (x_103 - x_104));
  param_3 = vec2((x_108 - x_109), (x_112 - x_113));
  float x_117 = param_2.x;
  float x_119 = param_3.y;
  float x_122 = param_3.x;
  float x_124 = param_2.y;
  float x_126 = ((x_117 * x_119) - (x_122 * x_124));
  x_67 = x_126;
  bool x_127 = (x_99 < 0.0f);
  bool x_129 = (x_127 & (x_126 < 0.0f));
  x_136_phi = x_129;
  if (!(x_129)) {
    x_135 = ((x_99 >= 0.0f) & (x_126 >= 0.0f));
    x_136_phi = x_135;
  }
  bool x_136 = x_136_phi;
  if (!(x_136)) {
    return 0;
  }
  float x_140 = p.x;
  float x_141 = c.x;
  float x_143 = p.y;
  float x_144 = c.y;
  float x_147 = a.x;
  float x_148 = c.x;
  float x_150 = a.y;
  float x_151 = c.y;
  param_4 = vec2((x_140 - x_141), (x_143 - x_144));
  param_5 = vec2((x_147 - x_148), (x_150 - x_151));
  float x_155 = param_4.x;
  float x_157 = param_5.y;
  float x_160 = param_5.x;
  float x_162 = param_4.y;
  float x_164 = ((x_155 * x_157) - (x_160 * x_162));
  x_66 = x_164;
  bool x_166 = (x_127 & (x_164 < 0.0f));
  x_173_phi = x_166;
  if (!(x_166)) {
    x_172 = ((x_99 >= 0.0f) & (x_164 >= 0.0f));
    x_173_phi = x_172;
  }
  bool x_173 = x_173_phi;
  if (!(x_173)) {
    return 0;
  }
  return 1;
}
void main_1() {
  vec2 param_6 = vec2(0.0f);
  vec2 param_7 = vec2(0.0f);
  vec2 param_8 = vec2(0.0f);
  vec2 param_9 = vec2(0.0f);
  vec4 x_55 = tint_symbol;
  vec2 x_58 = v.tint_symbol_3.resolution;
  param_6 = (vec2(x_55[0u], x_55[1u]) / x_58);
  param_7 = vec2(0.69999998807907104492f, 0.30000001192092895508f);
  param_8 = vec2(0.5f, 0.89999997615814208984f);
  param_9 = vec2(0.10000000149011611938f, 0.40000000596046447754f);
  int x_60 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_60 == 1)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  }
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:68: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:68: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
