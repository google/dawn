SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 resolution;
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 0) uniform buf0_1 {
  vec2 resolution;
} x_24;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

float cross2d_vf2_vf2_(inout vec2 a, inout vec2 b) {
  float x_76 = a.x;
  float x_78 = b.y;
  float x_81 = b.x;
  float x_83 = a.y;
  return ((x_76 * x_78) - (x_81 * x_83));
}

int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a_1, inout vec2 b_1, inout vec2 c) {
  float pab = 0.0f;
  vec2 param = vec2(0.0f, 0.0f);
  vec2 param_1 = vec2(0.0f, 0.0f);
  float pbc = 0.0f;
  vec2 param_2 = vec2(0.0f, 0.0f);
  vec2 param_3 = vec2(0.0f, 0.0f);
  float pca = 0.0f;
  vec2 param_4 = vec2(0.0f, 0.0f);
  vec2 param_5 = vec2(0.0f, 0.0f);
  bool x_145 = false;
  bool x_185 = false;
  bool x_146_phi = false;
  bool x_186_phi = false;
  float x_88 = p.x;
  float x_90 = a_1.x;
  float x_93 = p.y;
  float x_95 = a_1.y;
  float x_99 = b_1.x;
  float x_101 = a_1.x;
  float x_104 = b_1.y;
  float x_106 = a_1.y;
  param = vec2((x_88 - x_90), (x_93 - x_95));
  param_1 = vec2((x_99 - x_101), (x_104 - x_106));
  float x_109 = cross2d_vf2_vf2_(param, param_1);
  pab = x_109;
  float x_111 = p.x;
  float x_113 = b_1.x;
  float x_116 = p.y;
  float x_118 = b_1.y;
  float x_122 = c.x;
  float x_124 = b_1.x;
  float x_127 = c.y;
  float x_129 = b_1.y;
  param_2 = vec2((x_111 - x_113), (x_116 - x_118));
  param_3 = vec2((x_122 - x_124), (x_127 - x_129));
  float x_132 = cross2d_vf2_vf2_(param_2, param_3);
  pbc = x_132;
  bool x_137 = ((pab < 0.0f) & (pbc < 0.0f));
  x_146_phi = x_137;
  if (!(x_137)) {
    x_145 = ((pab >= 0.0f) & (pbc >= 0.0f));
    x_146_phi = x_145;
  }
  if (!(x_146_phi)) {
    return 0;
  }
  float x_151 = p.x;
  float x_153 = c.x;
  float x_156 = p.y;
  float x_158 = c.y;
  float x_162 = a_1.x;
  float x_164 = c.x;
  float x_167 = a_1.y;
  float x_169 = c.y;
  param_4 = vec2((x_151 - x_153), (x_156 - x_158));
  param_5 = vec2((x_162 - x_164), (x_167 - x_169));
  float x_172 = cross2d_vf2_vf2_(param_4, param_5);
  pca = x_172;
  bool x_177 = ((pab < 0.0f) & (pca < 0.0f));
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
  vec2 pos = vec2(0.0f, 0.0f);
  vec2 param_6 = vec2(0.0f, 0.0f);
  vec2 param_7 = vec2(0.0f, 0.0f);
  vec2 param_8 = vec2(0.0f, 0.0f);
  vec2 param_9 = vec2(0.0f, 0.0f);
  vec4 x_63 = tint_symbol;
  vec2 x_66 = x_24.resolution;
  pos = (vec2(x_63.x, x_63.y) / x_66);
  param_6 = pos;
  param_7 = vec2(0.699999988f, 0.300000012f);
  param_8 = vec2(0.5f, 0.899999976f);
  param_9 = vec2(0.100000001f, 0.400000006f);
  int x_69 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_69 == 1)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_4 {
  vec4 tint_symbol_2;
};
struct tint_symbol_5 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_6 = main_out(x_GLF_color);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_FragCoord;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:60: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:60: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



