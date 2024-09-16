SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
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
  int a = 0;
  int i = 0;
  int x_27 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  a = x_27;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  i = x_29;
  {
    while(true) {
      int x_34 = i;
      int x_36 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
      if ((x_34 < x_36)) {
      } else {
        break;
      }
      int x_39 = i;
      int x_42 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
      if ((tint_mod_i32(1, x_39) == x_42)) {
        {
          int x_48 = i;
          i = (x_48 + 1);
        }
        continue;
      }
      int x_46 = a;
      a = (x_46 + 1);
      {
        int x_48 = i;
        i = (x_48 + 1);
      }
      continue;
    }
  }
  int x_50 = a;
  int x_52 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  if ((x_50 == x_52)) {
    int x_58 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    int x_61 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_64 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_67 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    float v_2 = float(x_58);
    float v_3 = float(x_61);
    float v_4 = float(x_64);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_67));
  } else {
    int x_71 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    float x_72 = float(x_71);
    x_GLF_color = vec4(x_72, x_72, x_72, x_72);
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
