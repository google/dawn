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
  float x_76 = a.x;
  float x_78 = b.y;
  float x_81 = b.x;
  float x_83 = a.y;
  return ((x_76 * x_78) - (x_81 * x_83));
}
int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a_1, inout vec2 b_1, inout vec2 c) {
  float pab = 0.0f;
  vec2 param = vec2(0.0f);
  vec2 param_1 = vec2(0.0f);
  float pbc = 0.0f;
  vec2 param_2 = vec2(0.0f);
  vec2 param_3 = vec2(0.0f);
  float pca = 0.0f;
  vec2 param_4 = vec2(0.0f);
  vec2 param_5 = vec2(0.0f);
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
  float x_125 = pab;
  float x_127 = pbc;
  bool x_129 = ((x_125 < 0.0f) & (x_127 < 0.0f));
  x_138_phi = x_129;
  if (!(x_129)) {
    float x_133 = pab;
    float x_135 = pbc;
    x_137 = ((x_133 >= 0.0f) & (x_135 >= 0.0f));
    x_138_phi = x_137;
  }
  bool x_138 = x_138_phi;
  if (!(x_138)) {
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
  float x_157 = pab;
  float x_159 = pca;
  bool x_161 = ((x_157 < 0.0f) & (x_159 < 0.0f));
  x_170_phi = x_161;
  if (!(x_161)) {
    float x_165 = pab;
    float x_167 = pca;
    x_169 = ((x_165 >= 0.0f) & (x_167 >= 0.0f));
    x_170_phi = x_169;
  }
  bool x_170 = x_170_phi;
  if (!(x_170)) {
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
  vec4 x_63 = tint_symbol;
  vec2 x_66 = x_24.resolution;
  pos = (vec2(x_63[0u], x_63[1u]) / x_66);
  vec2 x_68 = pos;
  param_6 = x_68;
  param_7 = vec2(0.69999998807907104492f, 0.30000001192092895508f);
  param_8 = vec2(0.5f, 0.89999997615814208984f);
  param_9 = vec2(0.10000000149011611938f, 0.40000000596046447754f);
  int x_69 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_69 == 1)) {
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
