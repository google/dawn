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
  ivec2 x_80 = ivec2(0);
  int x_83 = 0;
  ivec2 x_113 = ivec2(0);
  ivec2 x_114 = ivec2(0);
  ivec2 x_116 = ivec2(0);
  int x_119 = 0;
  vec2 x_62 = (tint_symbol.xy / v.tint_symbol_3.resolution);
  int x_65 = tint_f32_to_i32((x_62[0u] * 8.0f));
  int x_69 = tint_f32_to_i32((x_62[1u] * 8.0f));
  x_80 = ivec2(((((x_65 & 5) | (x_69 & 10)) * 8) + ((x_69 & 5) | (x_65 & 10))), 0);
  x_83 = 0;
  {
    while(true) {
      ivec2 x_94 = ivec2(0);
      ivec2 x_95 = ivec2(0);
      ivec2 x_102 = ivec2(0);
      ivec2 x_103 = ivec2(0);
      int x_84 = 0;
      if ((x_83 < 100)) {
      } else {
        break;
      }
      x_95 = x_80;
      if ((x_80.x > 0)) {
        x_94 = x_80;
        x_94[1u] = (x_80.y - 1);
        x_95 = x_94;
      }
      x_103 = x_95;
      if ((x_95.x < 0)) {
        x_102 = x_95;
        x_102[1u] = (x_95.y + 1);
        x_103 = x_102;
      }
      ivec2 x_81_1 = x_103;
      int v_1 = x_103.x;
      x_81_1[0u] = (v_1 + tint_div_i32(x_103.y, 2));
      ivec2 x_81 = x_81_1;
      {
        x_84 = (x_83 + 1);
        x_80 = x_81;
        x_83 = x_84;
      }
      continue;
    }
  }
  int x_108 = x_80.x;
  x_114 = x_80;
  if ((x_108 < 0)) {
    x_113 = x_80;
    x_113[0u] = -(x_108);
    x_114 = x_113;
  }
  x_116 = x_114;
  {
    while(true) {
      ivec2 x_117 = ivec2(0);
      x_119 = x_116.x;
      if ((x_119 > 15)) {
      } else {
        break;
      }
      {
        x_117 = x_116;
        x_117[0u] = (x_119 - 16);
        x_116 = x_117;
      }
      continue;
    }
  }
  indexable = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
  x_GLF_color = indexable[x_119];
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
