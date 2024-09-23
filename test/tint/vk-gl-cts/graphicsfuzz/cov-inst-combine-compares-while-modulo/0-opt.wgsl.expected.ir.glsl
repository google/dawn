SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
void main_1() {
  int i = 0;
  int x_32 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  i = x_32;
  {
    while(true) {
      int x_37 = i;
      if ((x_37 >= 0)) {
      } else {
        break;
      }
      int x_40 = i;
      if ((tint_mod_i32(x_40, 2) == 0)) {
        int x_47 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
        int x_50 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
        int x_53 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
        float v_3 = float(x_47);
        float v_4 = float(x_50);
        x_GLF_color = vec4(1.0f, v_3, v_4, float(x_53));
      } else {
        float x_57 = v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el;
        x_GLF_color = vec4(x_57, x_57, x_57, x_57);
      }
      int x_59 = i;
      i = (x_59 - 1);
      {
      }
      continue;
    }
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
ERROR: 0:37: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
