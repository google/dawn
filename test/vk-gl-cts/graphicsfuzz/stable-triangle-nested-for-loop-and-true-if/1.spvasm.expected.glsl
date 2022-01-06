SKIP: FAILED

#version 310 es
precision mediump float;

struct buf1 {
  vec2 injectionSwitch;
};
struct buf0 {
  vec2 resolution;
};

layout (binding = 1) uniform buf1_1 {
  vec2 injectionSwitch;
} x_11;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 0) uniform buf0_1 {
  vec2 resolution;
} x_19;

int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a, inout vec2 b, inout vec2 c) {
  float x_78 = 0.0f;
  float x_79 = 0.0f;
  float x_80 = 0.0f;
  vec2 param = vec2(0.0f, 0.0f);
  vec2 param_1 = vec2(0.0f, 0.0f);
  vec2 param_2 = vec2(0.0f, 0.0f);
  vec2 param_3 = vec2(0.0f, 0.0f);
  vec2 param_4 = vec2(0.0f, 0.0f);
  vec2 param_5 = vec2(0.0f, 0.0f);
  bool x_147 = false;
  bool x_203 = false;
  bool x_148_phi = false;
  bool x_204_phi = false;
  float x_82 = p.x;
  float x_84 = a.x;
  float x_87 = p.y;
  float x_89 = a.y;
  float x_93 = b.x;
  float x_94 = a.x;
  float x_97 = b.y;
  float x_98 = a.y;
  param = vec2((x_82 - x_84), (x_87 - x_89));
  param_1 = vec2((x_93 - x_94), (x_97 - x_98));
  float x_102 = param.x;
  float x_104 = param_1.y;
  float x_107 = param_1.x;
  float x_109 = param.y;
  float x_111 = ((x_102 * x_104) - (x_107 * x_109));
  x_80 = x_111;
  float x_112 = p.x;
  float x_113 = b.x;
  float x_115 = p.y;
  float x_116 = b.y;
  float x_120 = c.x;
  float x_121 = b.x;
  float x_124 = c.y;
  float x_125 = b.y;
  param_2 = vec2((x_112 - x_113), (x_115 - x_116));
  param_3 = vec2((x_120 - x_121), (x_124 - x_125));
  float x_129 = param_2.x;
  float x_131 = param_3.y;
  float x_134 = param_3.x;
  float x_136 = param_2.y;
  float x_138 = ((x_129 * x_131) - (x_134 * x_136));
  x_79 = x_138;
  bool x_139 = (x_111 < 0.0f);
  bool x_141 = (x_139 & (x_138 < 0.0f));
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
      int x_153 = x_153_phi;
      float x_159 = x_11.injectionSwitch.y;
      int x_160 = int(x_159);
      if ((x_153 < x_160)) {
      } else {
        break;
      }
      x_GLF_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
      x_164_phi = 0;
      while (true) {
        int x_165 = 0;
        int x_164 = x_164_phi;
        if ((x_164 < x_160)) {
        } else {
          break;
        }
        x_GLF_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
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
  float x_171 = p.x;
  float x_172 = c.x;
  float x_174 = p.y;
  float x_175 = c.y;
  float x_178 = a.x;
  float x_179 = c.x;
  float x_181 = a.y;
  float x_182 = c.y;
  param_4 = vec2((x_171 - x_172), (x_174 - x_175));
  param_5 = vec2((x_178 - x_179), (x_181 - x_182));
  float x_186 = param_4.x;
  float x_188 = param_5.y;
  float x_191 = param_5.x;
  float x_193 = param_4.y;
  float x_195 = ((x_186 * x_188) - (x_191 * x_193));
  x_78 = x_195;
  bool x_197 = (x_139 & (x_195 < 0.0f));
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
  vec2 param_6 = vec2(0.0f, 0.0f);
  vec2 param_7 = vec2(0.0f, 0.0f);
  vec2 param_8 = vec2(0.0f, 0.0f);
  vec2 param_9 = vec2(0.0f, 0.0f);
  vec4 x_60 = tint_symbol;
  vec2 x_63 = x_19.resolution;
  param_6 = (vec2(x_60.x, x_60.y) / x_63);
  param_7 = vec2(0.699999988f, 0.300000012f);
  param_8 = vec2(0.5f, 0.899999976f);
  param_9 = vec2(0.100000001f, 0.400000006f);
  int x_65 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_65 == 1)) {
    float x_71 = x_11.injectionSwitch.y;
    float x_73 = x_11.injectionSwitch.x;
    if ((x_71 >= x_73)) {
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
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
ERROR: 0:67: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:67: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



