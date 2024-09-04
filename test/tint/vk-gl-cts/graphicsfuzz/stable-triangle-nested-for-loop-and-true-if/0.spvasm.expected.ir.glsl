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
uniform buf0 x_17;
vec4 x_GLF_color = vec4(0.0f);
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
  bool x_136 = false;
  bool x_172 = false;
  bool x_173 = false;
  float x_81 = b.x;
  float x_82 = a.x;
  float x_85 = b.y;
  float x_86 = a.y;
  param = vec2((p.x - a.x), (p.y - a.y));
  param_1 = vec2((x_81 - x_82), (x_85 - x_86));
  float x_99 = ((param.x * param_1.y) - (param_1.x * param.y));
  x_68 = x_99;
  float x_108 = c.x;
  float x_109 = b.x;
  float x_112 = c.y;
  float x_113 = b.y;
  param_2 = vec2((p.x - b.x), (p.y - b.y));
  param_3 = vec2((x_108 - x_109), (x_112 - x_113));
  float x_126 = ((param_2.x * param_3.y) - (param_3.x * param_2.y));
  x_67 = x_126;
  bool x_127 = (x_99 < 0.0f);
  bool x_129 = (x_127 & (x_126 < 0.0f));
  x_136 = x_129;
  if (!(x_129)) {
    x_135 = ((x_99 >= 0.0f) & (x_126 >= 0.0f));
    x_136 = x_135;
  }
  if (!(x_136)) {
    return 0;
  }
  float x_147 = a.x;
  float x_148 = c.x;
  float x_150 = a.y;
  float x_151 = c.y;
  param_4 = vec2((p.x - c.x), (p.y - c.y));
  param_5 = vec2((x_147 - x_148), (x_150 - x_151));
  float x_164 = ((param_4.x * param_5.y) - (param_5.x * param_4.y));
  x_66 = x_164;
  bool x_166 = (x_127 & (x_164 < 0.0f));
  x_173 = x_166;
  if (!(x_166)) {
    x_172 = ((x_99 >= 0.0f) & (x_164 >= 0.0f));
    x_173 = x_172;
  }
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
  param_6 = (tint_symbol.xy / x_17.resolution);
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
