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
  float x_32 = tint_symbol.x;
  int x_35 = v.tint_symbol_3.x_GLF_uniform_int_values[1].el;
  a = (((tint_f32_to_i32(x_32) < x_35)) ? (0) : (-1));
  i = 0;
  {
    while(true) {
      int x_42 = i;
      if ((x_42 < 5)) {
      } else {
        break;
      }
      int x_45 = a;
      a = tint_div_i32(x_45, 2);
      {
        int x_47 = i;
        i = (x_47 + 1);
      }
      continue;
    }
  }
  int x_49 = a;
  if ((x_49 == 0)) {
    int x_55 = v.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    int x_58 = v.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_61 = v.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    int x_64 = v.tint_symbol_3.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_55);
    float v_2 = float(x_58);
    float v_3 = float(x_61);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_64));
  } else {
    int x_68 = v.tint_symbol_3.x_GLF_uniform_int_values[1].el;
    float x_69 = float(x_68);
    x_GLF_color = vec4(x_69, x_69, x_69, x_69);
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
