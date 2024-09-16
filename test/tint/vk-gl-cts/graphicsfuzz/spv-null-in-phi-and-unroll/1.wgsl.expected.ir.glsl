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
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  vec4 x_77[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_78[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_79[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_80[16] = vec4[16](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_89 = vec4(0.0f);
  vec4 x_89_phi = vec4(0.0f);
  int x_92_phi = 0;
  vec4 x_81 = tint_symbol;
  vec2 x_84 = v.tint_symbol_3.resolution;
  vec2 x_87 = floor(((vec2(x_81[0u], x_81[1u]) / x_84) * 32.0f));
  x_89_phi = vec4(0.5f, 0.5f, 1.0f, 1.0f);
  x_92_phi = 0;
  {
    while(true) {
      vec4 x_136 = vec4(0.0f);
      int x_93 = 0;
      bool x_121_phi = false;
      vec4 x_90_phi = vec4(0.0f);
      x_89 = x_89_phi;
      int x_92 = x_92_phi;
      if ((x_92 < 8)) {
      } else {
        break;
      }
      vec4 x_98 = vec4(0.0f);
      x_77 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
      x_98 = x_77[x_92];
      switch(0u) {
        default:
        {
          float x_101 = x_87[0u];
          float x_102 = x_98.x;
          if ((x_101 < x_102)) {
            x_121_phi = false;
            break;
          }
          float x_106 = x_87[1u];
          float x_107 = x_98.y;
          if ((x_106 < x_107)) {
            x_121_phi = false;
            break;
          }
          if ((x_101 > (x_102 + x_98.z))) {
            x_121_phi = false;
            break;
          }
          if ((x_106 > (x_107 + x_98.w))) {
            x_121_phi = false;
            break;
          }
          x_121_phi = true;
          break;
        }
      }
      bool x_121 = x_121_phi;
      x_90_phi = x_89;
      if (x_121) {
        x_78 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_125 = x_78[x_92].x;
        x_79 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_128 = x_79[x_92].y;
        x_80 = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
        int v_2 = tint_f32_to_i32(x_125);
        x_136 = x_80[tint_mod_i32((((v_2 * tint_f32_to_i32(x_128)) + (x_92 * 9)) + 11), 16)];
        x_90_phi = x_136;
      }
      vec4 x_90 = x_90_phi;
      {
        x_93 = (x_92 + 1);
        x_89_phi = x_90;
        x_92_phi = x_93;
      }
      continue;
    }
  }
  x_GLF_color = x_89;
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
