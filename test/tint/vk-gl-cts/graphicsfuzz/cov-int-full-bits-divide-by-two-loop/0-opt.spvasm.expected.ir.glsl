SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
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
  int a = 0;
  int i = 0;
  int v_1 = tint_f32_to_i32(tint_symbol.x);
  a = (((v_1 < v.tint_symbol_3.x_GLF_uniform_int_values[1].el)) ? (0) : (-1));
  i = 0;
  {
    while(true) {
      if ((i < 5)) {
      } else {
        break;
      }
      a = tint_div_i32(a, 2);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((a == 0)) {
    float v_2 = float(v.tint_symbol_3.x_GLF_uniform_int_values[0].el);
    float v_3 = float(v.tint_symbol_3.x_GLF_uniform_int_values[1].el);
    float v_4 = float(v.tint_symbol_3.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(v.tint_symbol_3.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_3.x_GLF_uniform_int_values[1].el));
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
ERROR: 0:26: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:26: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:26: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
