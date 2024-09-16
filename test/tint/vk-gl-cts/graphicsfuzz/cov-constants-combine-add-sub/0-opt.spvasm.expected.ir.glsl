SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
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
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
int func_f1_(inout float f) {
  int a = 0;
  int b = 0;
  int i = 0;
  a = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  b = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  i = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  {
    while(true) {
      if ((i < v.tint_symbol_1.x_GLF_uniform_int_values[4].el)) {
      } else {
        break;
      }
      if ((a > v.tint_symbol_1.x_GLF_uniform_int_values[3].el)) {
        break;
      }
      int v_1 = tint_f32_to_i32(f);
      int v_2 = (v_1 - tint_div_i32(v.tint_symbol_1.x_GLF_uniform_int_values[1].el, 2));
      a = (v_2 + i);
      b = (b + 1);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((b == v.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
    int x_100 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    return x_100;
  } else {
    int x_102 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    return x_102;
  }
  /* unreachable */
}
void main_1() {
  float param = 0.0f;
  param = 0.69999998807907104492f;
  int x_34 = func_f1_(param);
  if ((x_34 == v.tint_symbol_1.x_GLF_uniform_int_values[1].el)) {
    float v_3 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    float v_5 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
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
