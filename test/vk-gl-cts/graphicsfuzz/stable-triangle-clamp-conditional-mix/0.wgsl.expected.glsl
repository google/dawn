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
  bool x_137 = false;
  bool x_169 = false;
  bool x_138_phi = false;
  bool x_170_phi = false;
  float x_88 = p.x;
  float x_90 = a_1.x;
  float x_93 = p.y;
  float x_95 = a_1.y;
  float x_99 = b_1.x;
  float x_100 = a_1.x;
  float x_103 = b_1.y;
  float x_104 = a_1.y;
  param = vec2((x_88 - x_90), (x_93 - x_95));
  param_1 = vec2((x_99 - x_100), (x_103 - x_104));
  float x_107 = cross2d_vf2_vf2_(param, param_1);
  pab = x_107;
  float x_108 = p.x;
  float x_109 = b_1.x;
  float x_111 = p.y;
  float x_112 = b_1.y;
  float x_116 = c.x;
  float x_117 = b_1.x;
  float x_120 = c.y;
  float x_121 = b_1.y;
  param_2 = vec2((x_108 - x_109), (x_111 - x_112));
  param_3 = vec2((x_116 - x_117), (x_120 - x_121));
  float x_124 = cross2d_vf2_vf2_(param_2, param_3);
  pbc = x_124;
  bool x_129 = ((pab < 0.0f) & (pbc < 0.0f));
  x_138_phi = x_129;
  if (!(x_129)) {
    x_137 = ((pab >= 0.0f) & (pbc >= 0.0f));
    x_138_phi = x_137;
  }
  if (!(x_138_phi)) {
    return 0;
  }
  float x_142 = p.x;
  float x_143 = c.x;
  float x_145 = p.y;
  float x_146 = c.y;
  float x_149 = a_1.x;
  float x_150 = c.x;
  float x_152 = a_1.y;
  float x_153 = c.y;
  param_4 = vec2((x_142 - x_143), (x_145 - x_146));
  param_5 = vec2((x_149 - x_150), (x_152 - x_153));
  float x_156 = cross2d_vf2_vf2_(param_4, param_5);
  pca = x_156;
  bool x_161 = ((pab < 0.0f) & (pca < 0.0f));
  x_170_phi = x_161;
  if (!(x_161)) {
    x_169 = ((pab >= 0.0f) & (pca >= 0.0f));
    x_170_phi = x_169;
  }
  if (!(x_170_phi)) {
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



