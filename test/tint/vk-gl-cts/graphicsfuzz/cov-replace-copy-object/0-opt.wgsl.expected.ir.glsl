SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  int one;
};

struct S {
  int data;
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
int func_struct_S_i11_i1_(inout S s, inout int x) {
  int x_17 = s.data;
  if ((x_17 == 1)) {
    int x_18 = x;
    int x_19 = s.data;
    return (x_18 + x_19);
  } else {
    int x_21 = x;
    return x_21;
  }
  /* unreachable */
}
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
void main_1() {
  int a = 0;
  S arr[1] = S[1](S(0));
  int i = 0;
  S param = S(0);
  int param_1 = 0;
  S param_2 = S(0);
  int param_3 = 0;
  a = 0;
  int x_22 = v.tint_symbol_1.one;
  arr[0].data = x_22;
  i = 0;
  {
    while(true) {
      int x_23 = i;
      int x_24 = v.tint_symbol_1.one;
      if ((x_23 < (5 + x_24))) {
      } else {
        break;
      }
      int x_26 = i;
      if ((tint_mod_i32(x_26, 2) != 0)) {
        S x_74 = arr[0];
        param = x_74;
        int x_28 = i;
        param_1 = x_28;
        int x_29 = func_struct_S_i11_i1_(param, param_1);
        S x_75 = param;
        arr[0] = x_75;
        a = x_29;
      } else {
        S x_78 = arr[0];
        param_2 = x_78;
        param_3 = 1;
        int x_30 = func_struct_S_i11_i1_(param_2, param_3);
        S x_79 = param_2;
        arr[0] = x_79;
        a = x_30;
      }
      {
        int x_31 = i;
        i = (x_31 + 1);
      }
      continue;
    }
  }
  int x_33 = a;
  if ((x_33 == 6)) {
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
ERROR: 0:37: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
