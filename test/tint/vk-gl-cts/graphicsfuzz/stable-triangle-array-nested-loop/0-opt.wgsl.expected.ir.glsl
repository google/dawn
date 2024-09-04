SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_24;
vec4 x_GLF_color = vec4(0.0f);
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
  bool x_168 = false;
  bool x_141_phi = false;
  bool x_169_phi = false;
  int x_173_phi = 0;
  switch(0u) {
    default:
    {
      float x_95 = p.x;
      float x_97 = a_1.x;
      float x_100 = p.y;
      float x_102 = a_1.y;
      float x_106 = b_1.x;
      float x_107 = a_1.x;
      float x_110 = b_1.y;
      float x_111 = a_1.y;
      param = vec2((x_95 - x_97), (x_100 - x_102));
      param_1 = vec2((x_106 - x_107), (x_110 - x_111));
      float x_114 = cross2d_vf2_vf2_(param, param_1);
      pab = x_114;
      float x_115 = p.x;
      float x_116 = b_1.x;
      float x_118 = p.y;
      float x_119 = b_1.y;
      float x_123 = c.x;
      float x_124 = b_1.x;
      float x_127 = c.y;
      float x_128 = b_1.y;
      param_2 = vec2((x_115 - x_116), (x_118 - x_119));
      param_3 = vec2((x_123 - x_124), (x_127 - x_128));
      float x_131 = cross2d_vf2_vf2_(param_2, param_3);
      pbc = x_131;
      bool x_134 = ((x_114 < 0.0f) & (x_131 < 0.0f));
      x_141_phi = x_134;
      if (!(x_134)) {
        x_140 = ((x_114 >= 0.0f) & (x_131 >= 0.0f));
        x_141_phi = x_140;
      }
      bool x_141 = x_141_phi;
      if (!(x_141)) {
        x_90 = true;
        x_91 = 0;
        x_173_phi = 0;
        break;
      }
      float x_145 = p.x;
      float x_146 = c.x;
      float x_148 = p.y;
      float x_149 = c.y;
      float x_152 = a_1.x;
      float x_153 = c.x;
      float x_155 = a_1.y;
      float x_156 = c.y;
      param_4 = vec2((x_145 - x_146), (x_148 - x_149));
      param_5 = vec2((x_152 - x_153), (x_155 - x_156));
      float x_159 = cross2d_vf2_vf2_(param_4, param_5);
      pca = x_159;
      bool x_162 = ((x_114 < 0.0f) & (x_159 < 0.0f));
      x_169_phi = x_162;
      if (!(x_162)) {
        x_168 = ((x_114 >= 0.0f) & (x_159 >= 0.0f));
        x_169_phi = x_168;
      }
      bool x_169 = x_169_phi;
      if (!(x_169)) {
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
  int x_173 = x_173_phi;
  return x_173;
}
void main_1() {
  vec2 pos = vec2(0.0f);
  vec2 param_6 = vec2(0.0f);
  vec2 param_7 = vec2(0.0f);
  vec2 param_8 = vec2(0.0f);
  vec2 param_9 = vec2(0.0f);
  vec4 x_67 = tint_symbol;
  vec2 x_70 = x_24.resolution;
  vec2 x_71 = (vec2(x_67[0u], x_67[1u]) / x_70);
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
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
