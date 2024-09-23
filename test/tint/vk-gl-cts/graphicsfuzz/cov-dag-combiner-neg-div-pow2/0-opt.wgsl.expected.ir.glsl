SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf1 {
  int ten;
};

struct buf0 {
  int minusEight;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  a = 0;
  b = 0;
  i = 0;
  {
    while(true) {
      int x_36 = i;
      int x_38 = v.tint_symbol_1.ten;
      if ((x_36 < x_38)) {
      } else {
        break;
      }
      int x_41 = a;
      if ((x_41 > 5)) {
        break;
      }
      int x_46 = v_1.tint_symbol_3.minusEight;
      int x_48 = a;
      a = (x_48 + tint_div_i32(x_46, -4));
      int x_50 = b;
      b = (x_50 + 1);
      {
        int x_52 = i;
        i = (x_52 + 1);
      }
      continue;
    }
  }
  int x_54 = b;
  if ((x_54 == 3)) {
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
ERROR: 0:29: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:29: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:29: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
