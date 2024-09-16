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
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
void main_1() {
  int count = 0;
  int i = 0;
  count = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  i = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < v.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      int v_2 = tint_mod_i32(count, -93448);
      if ((v_2 > v.tint_symbol_1.x_GLF_uniform_int_values[1].el)) {
        count = (count + 1);
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((count == v.tint_symbol_1.x_GLF_uniform_int_values[1].el)) {
    float v_3 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_5 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el));
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
