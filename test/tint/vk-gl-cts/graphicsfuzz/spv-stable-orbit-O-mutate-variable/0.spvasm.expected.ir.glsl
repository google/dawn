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
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  vec4 indexable[16] = vec4[16](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  ivec2 x_77 = ivec2(0);
  int x_80 = 0;
  ivec2 x_110 = ivec2(0);
  ivec2 x_111 = ivec2(0);
  ivec2 x_113 = ivec2(0);
  int x_116 = 0;
  vec2 x_60 = (tint_symbol.xy / v.tint_symbol_3.resolution);
  int x_63 = tint_f32_to_i32((x_60[0u] * 8.0f));
  int x_66 = tint_f32_to_i32((x_60[1u] * 8.0f));
  x_77 = ivec2(((((x_63 & 5) | (x_66 & 10)) * 8) + ((x_66 & 5) | (x_63 & 10))), 0);
  x_80 = 0;
  {
    while(true) {
      ivec2 x_91 = ivec2(0);
      ivec2 x_92 = ivec2(0);
      ivec2 x_99 = ivec2(0);
      ivec2 x_100 = ivec2(0);
      int x_81 = 0;
      if ((x_80 < 100)) {
      } else {
        break;
      }
      x_92 = x_77;
      if ((x_77.x > 0)) {
        x_91 = x_77;
        x_91[1u] = (x_77.y - 1);
        x_92 = x_91;
      }
      x_100 = x_92;
      if ((x_92.x < 0)) {
        x_99 = x_92;
        x_99[1u] = (x_92.y + 1);
        x_100 = x_99;
      }
      ivec2 x_78_1 = x_100;
      int v_1 = x_100.x;
      x_78_1[0u] = (v_1 + tint_div_i32(x_100.y, 2));
      ivec2 x_78 = x_78_1;
      {
        x_81 = (x_80 + 1);
        x_77 = x_78;
        x_80 = x_81;
      }
      continue;
    }
  }
  int x_105 = x_77.x;
  x_111 = x_77;
  if ((x_105 < 0)) {
    x_110 = ivec2(0);
    x_110[0u] = -(x_105);
    x_111 = x_110;
  }
  x_113 = x_111;
  {
    while(true) {
      ivec2 x_114 = ivec2(0);
      x_116 = x_113.x;
      if ((x_116 > 15)) {
      } else {
        break;
      }
      {
        x_114 = ivec2(0);
        x_114[0u] = (x_116 - 16);
        x_113 = x_114;
      }
      continue;
    }
  }
  indexable = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
  x_GLF_color = indexable[x_116];
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
ERROR: 0:22: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:22: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
