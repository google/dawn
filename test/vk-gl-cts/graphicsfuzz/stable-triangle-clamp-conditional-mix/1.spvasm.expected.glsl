SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 resolution;
};

layout (binding = 0) uniform buf0_1 {
  vec2 resolution;
} x_15;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

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
  vec2 param = vec2(0.0f, 0.0f);
  vec2 param_1 = vec2(0.0f, 0.0f);
  float pbc = 0.0f;
  vec2 param_2 = vec2(0.0f, 0.0f);
  vec2 param_3 = vec2(0.0f, 0.0f);
  float pca = 0.0f;
  vec2 param_4 = vec2(0.0f, 0.0f);
  vec2 param_5 = vec2(0.0f, 0.0f);
  bool x_173 = false;
  bool x_205 = false;
  bool x_174_phi = false;
  bool x_206_phi = false;
  float x_99 = x_15.resolution.x;
  float x_101 = x_15.resolution.y;
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
    vec2 x_117 = vec2(x_108.x, x_108.y);
    x_96 = x_107;
  } else {
    x_96 = -1.0f;
  }
  var_y = x_96;
  float x_120 = c.y;
  float x_121 = c.y;
  clamp_y = clamp(x_120, x_121, var_y);
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
  bool x_165 = ((pab < 0.0f) & (pbc < 0.0f));
  x_174_phi = x_165;
  if (!(x_165)) {
    x_173 = ((pab >= 0.0f) & (pbc >= 0.0f));
    x_174_phi = x_173;
  }
  if (!(x_174_phi)) {
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
  bool x_197 = ((pab < 0.0f) & (pca < 0.0f));
  x_206_phi = x_197;
  if (!(x_197)) {
    x_205 = ((pab >= 0.0f) & (pca >= 0.0f));
    x_206_phi = x_205;
  }
  if (!(x_206_phi)) {
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
  vec4 x_72 = tint_symbol;
  vec2 x_75 = x_15.resolution;
  pos = (vec2(x_72.x, x_72.y) / x_75);
  param_6 = pos;
  param_7 = vec2(0.699999988f, 0.300000012f);
  param_8 = vec2(0.5f, 0.899999976f);
  param_9 = vec2(0.100000001f, 0.400000006f);
  int x_78 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_78 == 1)) {
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
ERROR: 0:87: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:87: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



