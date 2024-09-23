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
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
void main_1() {
  int a = 0;
  int i = 0;
  int indexable[9] = int[9](0, 0, 0, 0, 0, 0, 0, 0, 0);
  int x_38 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  a = x_38;
  int x_40 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  i = x_40;
  {
    while(true) {
      int x_45 = i;
      int x_47 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
      if ((x_45 < x_47)) {
      } else {
        break;
      }
      int x_50 = i;
      int x_52 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
      indexable = int[9](1, 2, 3, 4, 5, 6, 7, 8, 9);
      int x_55 = indexable[tint_mod_i32(x_50, x_52)];
      int x_56 = a;
      a = (x_56 + x_55);
      {
        int x_58 = i;
        i = (x_58 + 1);
      }
      continue;
    }
  }
  int x_60 = a;
  int x_62 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  if ((x_60 == x_62)) {
    int x_68 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_71 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    int x_74 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    int x_77 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    float v_2 = float(x_68);
    float v_3 = float(x_71);
    float v_4 = float(x_74);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_77));
  } else {
    int x_81 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    float x_82 = float(x_81);
    x_GLF_color = vec4(x_82, x_82, x_82, x_82);
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
