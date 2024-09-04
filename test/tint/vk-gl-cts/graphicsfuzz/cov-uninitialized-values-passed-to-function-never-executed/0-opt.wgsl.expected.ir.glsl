SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct S {
  int data;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void func_struct_S_i11_i1_(inout S s, inout int x) {
  int x_103 = x_9.x_GLF_uniform_int_values[1].el;
  int x_105 = x_9.x_GLF_uniform_int_values[0].el;
  if ((x_103 == x_105)) {
    return;
  }
  int x_109 = x;
  s.data = x_109;
}
void main_1() {
  int i = 0;
  S arr[10] = S[10](S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0));
  int index = 0;
  S param = S(0);
  int param_1 = 0;
  S param_2 = S(0);
  int param_3 = 0;
  i = 0;
  {
    while(true) {
      int x_43 = i;
      if ((x_43 < 10)) {
      } else {
        break;
      }
      int x_46 = i;
      arr[x_46].data = 0;
      {
        int x_48 = i;
        i = (x_48 + 1);
      }
      continue;
    }
  }
  int x_51 = x_9.x_GLF_uniform_int_values[1].el;
  int x_53 = x_9.x_GLF_uniform_int_values[0].el;
  if ((x_51 == x_53)) {
    int x_58 = index;
    S x_60 = arr[x_58];
    param = x_60;
    int x_61 = index;
    param_1 = x_61;
    func_struct_S_i11_i1_(param, param_1);
    S x_63 = param;
    arr[x_58] = x_63;
  } else {
    int x_66 = x_9.x_GLF_uniform_int_values[0].el;
    S x_68 = arr[x_66];
    param_2 = x_68;
    int x_70 = x_9.x_GLF_uniform_int_values[1].el;
    param_3 = x_70;
    func_struct_S_i11_i1_(param_2, param_3);
    S x_72 = param_2;
    arr[x_66] = x_72;
  }
  int x_75 = x_9.x_GLF_uniform_int_values[0].el;
  int x_77 = arr[x_75].data;
  int x_79 = x_9.x_GLF_uniform_int_values[1].el;
  if ((x_77 == x_79)) {
    int x_85 = x_9.x_GLF_uniform_int_values[1].el;
    int x_88 = x_9.x_GLF_uniform_int_values[0].el;
    int x_91 = x_9.x_GLF_uniform_int_values[0].el;
    int x_94 = x_9.x_GLF_uniform_int_values[1].el;
    float v = float(x_85);
    float v_1 = float(x_88);
    float v_2 = float(x_91);
    x_GLF_color = vec4(v, v_1, v_2, float(x_94));
  } else {
    int x_98 = x_9.x_GLF_uniform_int_values[0].el;
    float x_99 = float(x_98);
    x_GLF_color = vec4(x_99, x_99, x_99, x_99);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
