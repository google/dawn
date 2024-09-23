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

layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
float cross2d_vf2_vf2_(inout vec2 a, inout vec2 b) {
  float x_85 = a.x;
  float x_87 = b.y;
  float x_90 = b.x;
  float x_92 = a.y;
  return ((x_85 * x_87) - (x_90 * x_92));
}
int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a_1, inout vec2 b_1, inout vec2 c) {
  float var_y = 0.0f;
  float x_96 = 0.0f;
  float x_97 = 0.0f;
  float clamp_y = 0.0f;
  float pab = 0.0f;
  vec2 param = vec2(0.0f);
  vec2 param_1 = vec2(0.0f);
  float pbc = 0.0f;
  vec2 param_2 = vec2(0.0f);
  vec2 param_3 = vec2(0.0f);
  float pca = 0.0f;
  vec2 param_4 = vec2(0.0f);
  vec2 param_5 = vec2(0.0f);
  bool x_173 = false;
  bool x_205 = false;
  bool x_174_phi = false;
  bool x_206_phi = false;
  float x_99 = v.tint_symbol_3.resolution.x;
  float x_101 = v.tint_symbol_3.resolution.y;
  if ((x_99 == x_101)) {
    float x_107 = c.y;
    vec2 x_108 = vec2(0.0f, x_107);
    if (true) {
      float x_112 = c.y;
      x_97 = x_112;
    } else {
      x_97 = 1.0f;
    }
    float x_113 = x_97;
    float x_114 = c.y;
    vec2 x_116 = vec2(1.0f, max(x_113, x_114));
    vec2 x_117 = vec2(x_108[0u], x_108[1u]);
    x_96 = x_107;
  } else {
    x_96 = -1.0f;
  }
  float x_118 = x_96;
  var_y = x_118;
  float x_120 = c.y;
  float x_121 = c.y;
  float x_122 = var_y;
  clamp_y = clamp(x_120, x_121, x_122);
  float x_125 = p.x;
  float x_127 = a_1.x;
  float x_130 = p.y;
  float x_132 = a_1.y;
  float x_136 = b_1.x;
  float x_137 = a_1.x;
  float x_140 = b_1.y;
  float x_141 = a_1.y;
  param = vec2((x_125 - x_127), (x_130 - x_132));
  param_1 = vec2((x_136 - x_137), (x_140 - x_141));
  float x_144 = cross2d_vf2_vf2_(param, param_1);
  pab = x_144;
  float x_145 = p.x;
  float x_146 = b_1.x;
  float x_148 = p.y;
  float x_149 = b_1.y;
  float x_153 = c.x;
  float x_154 = b_1.x;
  float x_156 = clamp_y;
  float x_157 = b_1.y;
  param_2 = vec2((x_145 - x_146), (x_148 - x_149));
  param_3 = vec2((x_153 - x_154), (x_156 - x_157));
  float x_160 = cross2d_vf2_vf2_(param_2, param_3);
  pbc = x_160;
  float x_161 = pab;
  float x_163 = pbc;
  bool x_165 = ((x_161 < 0.0f) & (x_163 < 0.0f));
  x_174_phi = x_165;
  if (!(x_165)) {
    float x_169 = pab;
    float x_171 = pbc;
    x_173 = ((x_169 >= 0.0f) & (x_171 >= 0.0f));
    x_174_phi = x_173;
  }
  bool x_174 = x_174_phi;
  if (!(x_174)) {
    return 0;
  }
  float x_178 = p.x;
  float x_179 = c.x;
  float x_181 = p.y;
  float x_182 = c.y;
  float x_185 = a_1.x;
  float x_186 = c.x;
  float x_188 = a_1.y;
  float x_189 = c.y;
  param_4 = vec2((x_178 - x_179), (x_181 - x_182));
  param_5 = vec2((x_185 - x_186), (x_188 - x_189));
  float x_192 = cross2d_vf2_vf2_(param_4, param_5);
  pca = x_192;
  float x_193 = pab;
  float x_195 = pca;
  bool x_197 = ((x_193 < 0.0f) & (x_195 < 0.0f));
  x_206_phi = x_197;
  if (!(x_197)) {
    float x_201 = pab;
    float x_203 = pca;
    x_205 = ((x_201 >= 0.0f) & (x_203 >= 0.0f));
    x_206_phi = x_205;
  }
  bool x_206 = x_206_phi;
  if (!(x_206)) {
    return 0;
  }
  return 1;
}
void main_1() {
  vec2 pos = vec2(0.0f);
  vec2 param_6 = vec2(0.0f);
  vec2 param_7 = vec2(0.0f);
  vec2 param_8 = vec2(0.0f);
  vec2 param_9 = vec2(0.0f);
  vec4 x_72 = tint_symbol;
  vec2 x_75 = v.tint_symbol_3.resolution;
  pos = (vec2(x_72[0u], x_72[1u]) / x_75);
  vec2 x_77 = pos;
  param_6 = x_77;
  param_7 = vec2(0.69999998807907104492f, 0.30000001192092895508f);
  param_8 = vec2(0.5f, 0.89999997615814208984f);
  param_9 = vec2(0.10000000149011611938f, 0.40000000596046447754f);
  int x_78 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_78 == 1)) {
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
ERROR: 0:97: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:97: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
