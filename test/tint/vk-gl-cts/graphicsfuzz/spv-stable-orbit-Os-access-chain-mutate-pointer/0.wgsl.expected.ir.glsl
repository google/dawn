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
  ivec2 x_76 = ivec2(0);
  ivec2 x_109 = ivec2(0);
  int x_115 = 0;
  ivec2 x_76_phi = ivec2(0);
  int x_79_phi = 0;
  ivec2 x_110_phi = ivec2(0);
  ivec2 x_112_phi = ivec2(0);
  vec4 x_55 = tint_symbol;
  vec2 x_58 = v.tint_symbol_3.resolution;
  vec2 x_59 = (vec2(x_55[0u], x_55[1u]) / x_58);
  int x_62 = tint_f32_to_i32((x_59[0u] * 8.0f));
  int x_65 = tint_f32_to_i32((x_59[1u] * 8.0f));
  ivec2 x_74 = ivec2(((((x_62 & 5) | (x_65 & 10)) * 8) + ((x_65 & 5) | (x_62 & 10))), 0);
  x_76_phi = x_74;
  x_79_phi = 0;
  {
    while(true) {
      ivec2 x_90 = ivec2(0);
      ivec2 x_98 = ivec2(0);
      int x_80 = 0;
      ivec2 x_91_phi = ivec2(0);
      ivec2 x_99_phi = ivec2(0);
      x_76 = x_76_phi;
      int x_79 = x_79_phi;
      if ((x_79 < 100)) {
      } else {
        break;
      }
      x_91_phi = x_76;
      if ((x_76.x > 0)) {
        x_90 = x_76;
        x_90[1u] = (x_76.y - 1);
        x_91_phi = x_90;
      }
      ivec2 x_91 = x_91_phi;
      x_99_phi = x_91;
      if ((x_91[0u] < 0)) {
        x_98 = x_91;
        x_98[1u] = (x_91[1u] + 1);
        x_99_phi = x_98;
      }
      ivec2 x_99 = x_99_phi;
      ivec2 x_77_1 = x_99;
      x_77_1[0u] = (x_99[0u] + tint_div_i32(x_99[1u], 2));
      ivec2 x_77 = x_77_1;
      {
        x_80 = (x_79 + 1);
        x_76_phi = x_77;
        x_79_phi = x_80;
      }
      continue;
    }
  }
  int x_104 = x_76.x;
  x_110_phi = x_76;
  if ((x_104 < 0)) {
    x_109 = x_76;
    x_109[0u] = -(x_104);
    x_110_phi = x_109;
  }
  ivec2 x_110 = x_110_phi;
  x_112_phi = x_110;
  {
    while(true) {
      ivec2 x_113 = ivec2(0);
      ivec2 x_112 = x_112_phi;
      x_115 = x_112[0u];
      if ((x_115 > 15)) {
      } else {
        break;
      }
      {
        x_113 = x_112;
        x_113[0u] = (x_115 - 16);
        x_112_phi = x_113;
      }
      continue;
    }
  }
  indexable = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
  vec4 x_120 = indexable[x_115];
  x_GLF_color = x_120;
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
