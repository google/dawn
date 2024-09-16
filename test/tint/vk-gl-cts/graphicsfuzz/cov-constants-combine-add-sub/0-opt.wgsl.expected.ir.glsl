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
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
int func_f1_(inout float f) {
  int a = 0;
  int b = 0;
  int i = 0;
  int x_60 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  a = x_60;
  int x_62 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  b = x_62;
  int x_64 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  i = x_64;
  {
    while(true) {
      int x_69 = i;
      int x_71 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
      if ((x_69 < x_71)) {
      } else {
        break;
      }
      int x_74 = a;
      int x_76 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
      if ((x_74 > x_76)) {
        break;
      }
      float x_80 = f;
      int x_83 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
      int x_86 = i;
      int v_1 = tint_f32_to_i32(x_80);
      a = ((v_1 - tint_div_i32(x_83, 2)) + x_86);
      int x_88 = b;
      b = (x_88 + 1);
      {
        int x_90 = i;
        i = (x_90 + 1);
      }
      continue;
    }
  }
  int x_92 = b;
  int x_94 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  if ((x_92 == x_94)) {
    int x_100 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    return x_100;
  } else {
    int x_102 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    return x_102;
  }
  /* unreachable */
}
void main_1() {
  float param = 0.0f;
  param = 0.69999998807907104492f;
  int x_34 = func_f1_(param);
  int x_36 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  if ((x_34 == x_36)) {
    int x_42 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_45 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_48 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_51 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float v_2 = float(x_42);
    float v_3 = float(x_45);
    float v_4 = float(x_48);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_51));
  } else {
    int x_55 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    float x_56 = float(x_55);
    x_GLF_color = vec4(x_56, x_56, x_56, x_56);
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
