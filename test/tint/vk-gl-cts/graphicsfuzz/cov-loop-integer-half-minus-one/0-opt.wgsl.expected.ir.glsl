SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void main_1() {
  int a = 0;
  int x_25 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  a = x_25;
  {
    while(true) {
      int x_30 = a;
      if ((x_30 >= 0)) {
      } else {
        break;
      }
      int x_33 = a;
      a = (tint_div_i32(x_33, 2) - 1);
      {
      }
      continue;
    }
  }
  int x_36 = a;
  int x_38 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  if ((x_36 == -(x_38))) {
    int x_45 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_48 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_51 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_54 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_45);
    float v_2 = float(x_48);
    float v_3 = float(x_51);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_54));
  } else {
    int x_58 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float x_59 = float(x_58);
    x_GLF_color = vec4(x_59, x_59, x_59, x_59);
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:25: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:25: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:25: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
