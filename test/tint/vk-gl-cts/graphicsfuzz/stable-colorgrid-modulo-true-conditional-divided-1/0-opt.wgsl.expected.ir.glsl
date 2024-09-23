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

layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float x_91 = 0.0f;
  float x_91_phi = 0.0f;
  int x_94_phi = 0;
  x_91_phi = -0.5f;
  x_94_phi = 1;
  {
    while(true) {
      float x_104 = 0.0f;
      float x_113 = 0.0f;
      int x_95 = 0;
      float x_92_phi = 0.0f;
      x_91 = x_91_phi;
      int x_94 = x_94_phi;
      if ((x_94 < 800)) {
      } else {
        break;
      }
      float x_112 = 0.0f;
      float x_113_phi = 0.0f;
      if ((tint_mod_i32(x_94, 32) == 0)) {
        x_104 = (x_91 + 0.40000000596046447754f);
        x_92_phi = x_104;
      } else {
        float x_106 = thirty_two;
        x_113_phi = x_91;
        float v_2 = float(x_94);
        float v_3 = round(x_106);
        float v_4 = float(x_94);
        if (((v_2 - (v_3 * floor((v_4 / round(x_106))))) <= 0.00999999977648258209f)) {
          x_112 = (x_91 + 100.0f);
          x_113_phi = x_112;
        }
        x_113 = x_113_phi;
        x_92_phi = x_113;
      }
      float x_92 = 0.0f;
      x_92 = x_92_phi;
      float x_115 = limit;
      if ((float(x_94) >= x_115)) {
        return x_92;
      }
      {
        x_95 = (x_94 + 1);
        x_91_phi = x_92;
        x_94_phi = x_95;
      }
      continue;
    }
  }
  return x_91;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int x_68_phi = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_52 = v.tint_symbol_3.resolution.x;
  float x_54 = round((x_52 * 0.125f));
  float x_56 = tint_symbol.x;
  param = x_56;
  param_1 = x_54;
  float x_57 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_57;
  float x_60 = tint_symbol.y;
  param_2 = x_60;
  param_3 = x_54;
  float x_61 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_61;
  float x_63 = c.x;
  float x_64 = c.y;
  c[2u] = (x_63 + x_64);
  x_68_phi = 0;
  {
    while(true) {
      int x_69 = 0;
      int x_68 = x_68_phi;
      if ((x_68 < 3)) {
      } else {
        break;
      }
      float x_75 = c[x_68];
      if ((x_75 >= 1.0f)) {
        float x_79 = c[x_68];
        float x_80 = c[x_68];
        c[x_68] = (x_79 * x_80);
      }
      {
        x_69 = (x_68 + 1);
        x_68_phi = x_69;
      }
      continue;
    }
  }
  vec3 x_82 = c;
  vec3 x_84 = normalize(abs(x_82));
  x_GLF_color = vec4(x_84[0u], x_84[1u], x_84[2u], 1.0f);
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
