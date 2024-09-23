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
  vec4 x_81[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_82[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_83[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_84[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_85[16] = vec4[16](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 x_95 = vec4(0.0f);
  int x_98 = 0;
  x_81 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
  vec4 x_86[8] = x_81;
  vec2 x_93 = floor(((tint_symbol.xy / v.tint_symbol_3.resolution) * 32.0f));
  x_95 = vec4(0.5f, 0.5f, 1.0f, 1.0f);
  x_98 = 0;
  {
    while(true) {
      bool x_127 = false;
      vec4 x_142 = vec4(0.0f);
      vec4 x_96 = vec4(0.0f);
      int x_99 = 0;
      if ((x_98 < 8)) {
      } else {
        break;
      }
      vec4 x_104 = vec4(0.0f);
      x_82 = x_86;
      x_104 = x_82[x_98];
      switch(0u) {
        default:
        {
          float x_107 = x_93[0u];
          float x_108 = x_104.x;
          if ((x_107 < x_108)) {
            x_127 = false;
            break;
          }
          float x_112 = x_93[1u];
          float x_113 = x_104.y;
          if ((x_112 < x_113)) {
            x_127 = false;
            break;
          }
          if ((x_107 > (x_108 + x_104.z))) {
            x_127 = false;
            break;
          }
          if ((x_112 > (x_113 + x_104.w))) {
            x_127 = false;
            break;
          }
          x_127 = true;
          break;
        }
      }
      x_96 = x_95;
      if (x_127) {
        x_83 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_131 = x_83[x_98].x;
        x_84 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_134 = x_84[x_98].y;
        x_85 = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
        int v_2 = tint_f32_to_i32(x_131);
        int v_3 = (v_2 * tint_f32_to_i32(x_134));
        x_142 = x_85[tint_mod_i32(((v_3 + (x_98 * 9)) + 11), 16)];
        x_96 = x_142;
      }
      {
        x_99 = (x_98 + 1);
        x_95 = x_96;
        x_98 = x_99;
      }
      continue;
    }
  }
  x_GLF_color = x_95;
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
