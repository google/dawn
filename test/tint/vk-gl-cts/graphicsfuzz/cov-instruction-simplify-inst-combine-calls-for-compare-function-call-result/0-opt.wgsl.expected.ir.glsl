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
  int x_16 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  i = x_16;
  {
    while(true) {
      int x_17 = i;
      int x_18 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
      if ((x_17 < x_18)) {
      } else {
        break;
      }
      int x_19 = i;
      int x_20 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
      if ((x_19 > x_20)) {
        int x_21 = a;
        return x_21;
      }
      {
        int x_22 = i;
        i = (x_22 + 1);
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
  int x_26 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  r[x_25] = x_26;
  int x_27 = v.tint_symbol_1.x_GLF_uniform_int_values[11].el;
  int x_28 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  r[x_27] = x_28;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_30 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  r[x_29] = x_30;
  int x_31 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_32 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  r[x_31] = x_32;
  int x_33 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  int x_34 = v.tint_symbol_1.x_GLF_uniform_int_values[5].el;
  r[x_33] = x_34;
  int x_35 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  int x_36 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
  r[x_35] = x_36;
  int x_37 = v.tint_symbol_1.x_GLF_uniform_int_values[5].el;
  int x_38 = v.tint_symbol_1.x_GLF_uniform_int_values[7].el;
  r[x_37] = x_38;
  int x_39 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
  int x_40 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
  r[x_39] = x_40;
  int x_41 = v.tint_symbol_1.x_GLF_uniform_int_values[9].el;
  int x_42 = v.tint_symbol_1.x_GLF_uniform_int_values[9].el;
  r[x_41] = x_42;
  int x_43 = v.tint_symbol_1.x_GLF_uniform_int_values[10].el;
  int x_44 = v.tint_symbol_1.x_GLF_uniform_int_values[10].el;
  r[x_43] = x_44;
  int x_45 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  i_1 = x_45;
  {
    while(true) {
      int x_46 = i_1;
      int x_47 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
      if ((x_46 < x_47)) {
      } else {
        break;
      }
      int x_48 = i_1;
      int x_49 = i_1;
      a_1[x_48] = x_49;
      int x_50 = i_1;
      int x_51 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
      int x_52 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
      if ((x_50 < tint_div_i32(x_51, x_52))) {
        int x_54 = i_1;
        int x_55 = i_1;
        int x_56 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
        a_1[x_54] = (x_55 + x_56);
        int x_58 = i_1;
        int x_59 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
        if ((x_58 < x_59)) {
          {
            int x_79 = i_1;
            i_1 = (x_79 + 1);
          }
          continue;
        }
        int x_60 = i_1;
        int x_61 = i_1;
        int x_62 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
        a_1[x_60] = (x_61 + x_62);
        int x_64 = i_1;
        int x_65 = a_1[x_64];
        param = x_65;
        int x_66 = f_i1_(param);
        int x_67 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
        if ((x_66 < x_67)) {
          int x_68 = i_1;
          int x_182_save = x_68;
          int x_69 = a_1[x_182_save];
          a_1[x_182_save] = (x_69 - 1);
        }
      } else {
        int x_71 = i_1;
        int x_72 = a_1[x_71];
        param_1 = x_72;
        int x_73 = f_i1_(param_1);
        int x_74 = v.tint_symbol_1.x_GLF_uniform_int_values[8].el;
        if ((x_73 < x_74)) {
          int x_75 = i_1;
          int x_76 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
          int x_77 = a_1[x_75];
          a_1[x_75] = (x_77 + x_76);
        }
      }
      {
        int x_79 = i_1;
        i_1 = (x_79 + 1);
      }
      continue;
    }
  }
  int x_81 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  i_2 = x_81;
  {
    while(true) {
      int x_82 = i_2;
      int x_83 = v.tint_symbol_1.x_GLF_uniform_int_values[6].el;
      if ((x_82 < x_83)) {
      } else {
        break;
      }
      int x_84 = i_2;
      int x_85 = a_1[x_84];
      int x_86 = i_2;
      int x_87 = r[x_86];
      if ((x_85 != x_87)) {
        int x_88 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
        float x_205 = float(x_88);
        x_GLF_color = vec4(x_205, x_205, x_205, x_205);
        return;
      }
      {
        int x_89 = i_2;
        i_2 = (x_89 + 1);
      }
      continue;
    }
  }
  int x_91 = v.tint_symbol_1.x_GLF_uniform_int_values[11].el;
  int x_92 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_93 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_94 = v.tint_symbol_1.x_GLF_uniform_int_values[11].el;
  float v_1 = float(x_91);
  float v_2 = float(x_92);
  float v_3 = float(x_93);
  x_GLF_color = vec4(v_1, v_2, v_3, float(x_94));
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:53: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:53: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:53: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
