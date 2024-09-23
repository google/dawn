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
float cross2d_vf2_vf2_(inout vec2 a, inout vec2 b) {
  float x_79 = a.x;
  float x_81 = b.y;
  float x_84 = b.x;
  float x_86 = a.y;
  return ((x_79 * x_81) - (x_84 * x_86));
}
int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a_1, inout vec2 b_1, inout vec2 c) {
  bool x_90 = false;
  int x_91 = 0;
  float pab = 0.0f;
  vec2 param = vec2(0.0f);
  vec2 param_1 = vec2(0.0f);
  float pbc = 0.0f;
  vec2 param_2 = vec2(0.0f);
  vec2 param_3 = vec2(0.0f);
  float pca = 0.0f;
  vec2 param_4 = vec2(0.0f);
  vec2 param_5 = vec2(0.0f);
  bool x_140 = false;
  bool x_141 = false;
  bool x_168 = false;
  bool x_169 = false;
  int x_173 = 0;
  switch(0u) {
    default:
    {
      float x_106 = b_1.x;
      float x_107 = a_1.x;
      float x_110 = b_1.y;
      float x_111 = a_1.y;
      param = vec2((p.x - a_1.x), (p.y - a_1.y));
      param_1 = vec2((x_106 - x_107), (x_110 - x_111));
      float x_114 = cross2d_vf2_vf2_(param, param_1);
      pab = x_114;
      float x_123 = c.x;
      float x_124 = b_1.x;
      float x_127 = c.y;
      float x_128 = b_1.y;
      param_2 = vec2((p.x - b_1.x), (p.y - b_1.y));
      param_3 = vec2((x_123 - x_124), (x_127 - x_128));
      float x_131 = cross2d_vf2_vf2_(param_2, param_3);
      pbc = x_131;
      bool x_134 = ((x_114 < 0.0f) & (x_131 < 0.0f));
      x_141 = x_134;
      if (!(x_134)) {
        x_140 = ((x_114 >= 0.0f) & (x_131 >= 0.0f));
        x_141 = x_140;
      }
      if (!(x_141)) {
        x_90 = true;
        x_91 = 0;
        x_173 = 0;
        break;
      }
      float x_152 = a_1.x;
      float x_153 = c.x;
      float x_155 = a_1.y;
      float x_156 = c.y;
      param_4 = vec2((p.x - c.x), (p.y - c.y));
      param_5 = vec2((x_152 - x_153), (x_155 - x_156));
      float x_159 = cross2d_vf2_vf2_(param_4, param_5);
      pca = x_159;
      bool x_162 = ((x_114 < 0.0f) & (x_159 < 0.0f));
      x_169 = x_162;
      if (!(x_162)) {
        x_168 = ((x_114 >= 0.0f) & (x_159 >= 0.0f));
        x_169 = x_168;
      }
      if (!(x_169)) {
        x_90 = true;
        x_91 = 0;
        x_173 = 0;
        break;
      }
      x_90 = true;
      x_91 = 1;
      x_173 = 1;
      break;
    }
  }
  return x_173;
}
void main_1() {
  vec2 pos = vec2(0.0f);
  vec2 param_6 = vec2(0.0f);
  vec2 param_7 = vec2(0.0f);
  vec2 param_8 = vec2(0.0f);
  vec2 param_9 = vec2(0.0f);
  vec2 x_71 = (tint_symbol.xy / v.tint_symbol_3.resolution);
  pos = x_71;
  param_6 = x_71;
  param_7 = vec2(0.69999998807907104492f, 0.30000001192092895508f);
  param_8 = vec2(0.5f, 0.89999997615814208984f);
  param_9 = vec2(0.10000000149011611938f, 0.40000000596046447754f);
  int x_72 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_72 == 1)) {
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
ERROR: 0:64: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:64: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
