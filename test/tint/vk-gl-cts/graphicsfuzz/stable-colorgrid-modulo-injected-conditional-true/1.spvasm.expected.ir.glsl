SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct buf1 {
  vec2 injectionSwitch;
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
layout(binding = 1, std140)
uniform tint_symbol_6_1_ubo {
  buf1 tint_symbol_5;
} v_1;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    while(true) {
      if ((i < 800)) {
      } else {
        break;
      }
      if ((tint_mod_i32(i, 32) == 0)) {
        result = (result + 0.40000000596046447754f);
      } else {
        float x_138 = thirty_two;
        float v_3 = float(i);
        float v_4 = round(x_138);
        float v_5 = float(i);
        if (((v_3 - (v_4 * floor((v_5 / round(x_138))))) <= 0.00999999977648258209f)) {
          result = (result + 100.0f);
        }
      }
      float v_6 = float(i);
      if ((v_6 >= limit)) {
        float x_152 = result;
        return x_152;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float x_155 = result;
  return x_155;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  vec3 x_58 = vec3(0.0f);
  c = vec3(7.0f, 8.0f, 9.0f);
  thirty_two_1 = round((v.tint_symbol_3.resolution.x / 8.0f));
  param = tint_symbol.x;
  param_1 = thirty_two_1;
  float x_66 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_66;
  param_2 = tint_symbol.y;
  param_3 = thirty_two_1;
  float x_71 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_71;
  c[2u] = (c.x + c.y);
  i_1 = 0;
  {
    while(true) {
      if ((i_1 < 3)) {
      } else {
        break;
      }
      if ((c[i_1] >= 1.0f)) {
        int x_92 = i_1;
        c[x_92] = (c[i_1] * c[i_1]);
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  if ((v_1.tint_symbol_5.injectionSwitch.x < v_1.tint_symbol_5.injectionSwitch.y)) {
    x_58 = abs(c);
  } else {
    x_58 = c;
  }
  vec3 x_115 = normalize(x_58);
  x_GLF_color = vec4(x_115[0u], x_115[1u], x_115[2u], 1.0f);
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
ERROR: 0:30: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:30: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:30: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
