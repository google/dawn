SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf1 {
  vec2 injectionSwitch;
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_1;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
int pointInTriangle_vf2_vf2_vf2_vf2_(inout vec2 p, inout vec2 a, inout vec2 b, inout vec2 c) {
  float x_78 = 0.0f;
  float x_79 = 0.0f;
  float x_80 = 0.0f;
  vec2 param = vec2(0.0f);
  vec2 param_1 = vec2(0.0f);
  vec2 param_2 = vec2(0.0f);
  vec2 param_3 = vec2(0.0f);
  vec2 param_4 = vec2(0.0f);
  vec2 param_5 = vec2(0.0f);
  bool x_147 = false;
  bool x_148 = false;
  bool x_203 = false;
  bool x_204 = false;
  float x_93 = b.x;
  float x_94 = a.x;
  float x_97 = b.y;
  float x_98 = a.y;
  param = vec2((p.x - a.x), (p.y - a.y));
  param_1 = vec2((x_93 - x_94), (x_97 - x_98));
  float x_111 = ((param.x * param_1.y) - (param_1.x * param.y));
  x_80 = x_111;
  float x_120 = c.x;
  float x_121 = b.x;
  float x_124 = c.y;
  float x_125 = b.y;
  param_2 = vec2((p.x - b.x), (p.y - b.y));
  param_3 = vec2((x_120 - x_121), (x_124 - x_125));
  float x_138 = ((param_2.x * param_3.y) - (param_3.x * param_2.y));
  x_79 = x_138;
  bool x_139 = (x_111 < 0.0f);
  bool x_141 = (x_139 & (x_138 < 0.0f));
  x_148 = x_141;
  if (!(x_141)) {
    x_147 = ((x_111 >= 0.0f) & (x_138 >= 0.0f));
    x_148 = x_147;
  }
  int x_153 = 0;
  if (!(x_148)) {
    x_153 = 0;
    {
      while(true) {
        int x_164 = 0;
        int x_154 = 0;
        int x_160 = tint_f32_to_i32(v.tint_symbol_3.injectionSwitch.y);
        if ((x_153 < x_160)) {
        } else {
          break;
        }
        x_GLF_color = vec4(1.0f);
        x_164 = 0;
        {
          while(true) {
            int x_165 = 0;
            if ((x_164 < x_160)) {
            } else {
              break;
            }
            x_GLF_color = vec4(1.0f);
            {
              x_165 = (x_164 + 1);
              x_164 = x_165;
            }
            continue;
          }
        }
        {
          x_154 = (x_153 + 1);
          x_153 = x_154;
        }
        continue;
      }
    }
    return 0;
  }
  float x_178 = a.x;
  float x_179 = c.x;
  float x_181 = a.y;
  float x_182 = c.y;
  param_4 = vec2((p.x - c.x), (p.y - c.y));
  param_5 = vec2((x_178 - x_179), (x_181 - x_182));
  float x_195 = ((param_4.x * param_5.y) - (param_5.x * param_4.y));
  x_78 = x_195;
  bool x_197 = (x_139 & (x_195 < 0.0f));
  x_204 = x_197;
  if (!(x_197)) {
    x_203 = ((x_111 >= 0.0f) & (x_195 >= 0.0f));
    x_204 = x_203;
  }
  if (!(x_204)) {
    return 0;
  }
  return 1;
}
void main_1() {
  vec2 param_6 = vec2(0.0f);
  vec2 param_7 = vec2(0.0f);
  vec2 param_8 = vec2(0.0f);
  vec2 param_9 = vec2(0.0f);
  param_6 = (tint_symbol.xy / v_1.tint_symbol_5.resolution);
  param_7 = vec2(0.69999998807907104492f, 0.30000001192092895508f);
  param_8 = vec2(0.5f, 0.89999997615814208984f);
  param_9 = vec2(0.10000000149011611938f, 0.40000000596046447754f);
  int x_65 = pointInTriangle_vf2_vf2_vf2_vf2_(param_6, param_7, param_8, param_9);
  if ((x_65 == 1)) {
    if ((v.tint_symbol_3.injectionSwitch.y >= v.tint_symbol_3.injectionSwitch.x)) {
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
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
ERROR: 0:63: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:63: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
