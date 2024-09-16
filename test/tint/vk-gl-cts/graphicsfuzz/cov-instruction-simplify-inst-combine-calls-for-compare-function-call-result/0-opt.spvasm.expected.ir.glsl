SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[12];
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
int f_i1_(inout int a) {
  int i = 0;
  i = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < v.tint_symbol_1.x_GLF_uniform_int_values[6].el)) {
      } else {
        break;
      }
      if ((i > v.tint_symbol_1.x_GLF_uniform_int_values[2].el)) {
        int x_21 = a;
        return x_21;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  int x_24 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  return x_24;
}
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void main_1() {
  int r[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i_1 = 0;
  int a_1[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int param = 0;
  int param_1 = 0;
  int i_2 = 0;
  int x_25 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  r[x_25] = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_27 = v.tint_symbol_1.x_GLF_uniform_int_values[11].el;
  r[x_27] = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  r[x_29] = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  int x_31 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  r[x_31] = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  int x_33 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  r[x_33] = v.tint_symbol_1.x_GLF_uniform_int_values[5].el;
  int x_35 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  r[x_35] = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
  int x_37 = v.tint_symbol_1.x_GLF_uniform_int_values[5].el;
  r[x_37] = v.tint_symbol_1.x_GLF_uniform_int_values[7].el;
  int x_39 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
  r[x_39] = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
  int x_41 = v.tint_symbol_1.x_GLF_uniform_int_values[9].el;
  r[x_41] = v.tint_symbol_1.x_GLF_uniform_int_values[9].el;
  int x_43 = v.tint_symbol_1.x_GLF_uniform_int_values[10].el;
  r[x_43] = v.tint_symbol_1.x_GLF_uniform_int_values[10].el;
  i_1 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i_1 < v.tint_symbol_1.x_GLF_uniform_int_values[6].el)) {
      } else {
        break;
      }
      int x_48 = i_1;
      a_1[x_48] = i_1;
      int v_1 = i_1;
      if ((v_1 < tint_div_i32(v.tint_symbol_1.x_GLF_uniform_int_values[6].el, v.tint_symbol_1.x_GLF_uniform_int_values[1].el))) {
        int x_54 = i_1;
        a_1[x_54] = (i_1 + v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
        if ((i_1 < v.tint_symbol_1.x_GLF_uniform_int_values[6].el)) {
          {
            i_1 = (i_1 + 1);
          }
          continue;
        }
        int x_60 = i_1;
        a_1[x_60] = (i_1 + v.tint_symbol_1.x_GLF_uniform_int_values[8].el);
        param = a_1[i_1];
        int x_66 = f_i1_(param);
        if ((x_66 < v.tint_symbol_1.x_GLF_uniform_int_values[8].el)) {
          int x_68 = i_1;
          int x_182_save = x_68;
          a_1[x_182_save] = (a_1[x_68] - 1);
        }
      } else {
        param_1 = a_1[i_1];
        int x_73 = f_i1_(param_1);
        if ((x_73 < v.tint_symbol_1.x_GLF_uniform_int_values[8].el)) {
          int x_75 = i_1;
          a_1[x_75] = (a_1[i_1] + v.tint_symbol_1.x_GLF_uniform_int_values[4].el);
        }
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  i_2 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i_2 < v.tint_symbol_1.x_GLF_uniform_int_values[6].el)) {
      } else {
        break;
      }
      if ((a_1[i_2] != r[i_2])) {
        x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el));
        return;
      }
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  float v_2 = float(v.tint_symbol_1.x_GLF_uniform_int_values[11].el);
  float v_3 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  x_GLF_color = vec4(v_2, v_3, v_4, float(v.tint_symbol_1.x_GLF_uniform_int_values[11].el));
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:47: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:47: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:47: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
