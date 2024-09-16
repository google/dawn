SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  int two;
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
  int i = 0;
  int r = 0;
  i = 0;
  r = 0;
  {
    while(true) {
      int x_35 = r;
      int x_37 = v.tint_symbol_1.two;
      if ((x_35 < (x_37 * 4))) {
      } else {
        break;
      }
      int x_41 = r;
      int x_43 = v.tint_symbol_1.two;
      int x_46 = i;
      i = (x_46 + ivec4(1, 2, 3, 4)[tint_div_i32(x_41, x_43)]);
      {
        int x_48 = r;
        r = (x_48 + 2);
      }
      continue;
    }
  }
  int x_50 = i;
  if ((x_50 == 10)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
ERROR: 0:21: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:21: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:21: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
