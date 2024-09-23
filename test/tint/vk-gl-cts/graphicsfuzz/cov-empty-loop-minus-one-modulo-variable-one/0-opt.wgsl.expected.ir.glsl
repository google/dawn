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
  int arr[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int a = 0;
  int i = 0;
  arr = int[10](1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  a = 0;
  int x_42 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_44 = arr[x_42];
  if ((x_44 == 2)) {
    int x_49 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    i = x_49;
    {
      while(true) {
        int x_54 = i;
        int x_56 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
        if ((x_54 < x_56)) {
        } else {
          break;
        }
        {
          int x_59 = i;
          i = (x_59 + 1);
        }
        continue;
      }
    }
    int x_61 = a;
    a = (x_61 + 1);
  }
  int x_63 = a;
  int x_66 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  if ((tint_mod_i32(-1, x_63) == x_66)) {
    int x_71 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_75 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    arr[ivec2(x_71, x_71)[1u]] = x_75;
  }
  int x_78 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_80 = arr[x_78];
  int x_82 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  if ((x_80 == x_82)) {
    int x_88 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_91 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_94 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_97 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float v_2 = float(x_88);
    float v_3 = float(x_91);
    float v_4 = float(x_94);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_97));
  } else {
    int x_101 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    float x_102 = float(x_101);
    x_GLF_color = vec4(x_102, x_102, x_102, x_102);
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
