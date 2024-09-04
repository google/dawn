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
  if ((x_9.x_GLF_uniform_int_values[1].el == x_9.x_GLF_uniform_int_values[0].el)) {
    return;
  }
  s.data = x;
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
      if ((i < 10)) {
      } else {
        break;
      }
      int x_46 = i;
      arr[x_46].data = 0;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((x_9.x_GLF_uniform_int_values[1].el == x_9.x_GLF_uniform_int_values[0].el)) {
    int x_58 = index;
    param = arr[index];
    param_1 = index;
    func_struct_S_i11_i1_(param, param_1);
    arr[x_58] = param;
  } else {
    int x_66 = x_9.x_GLF_uniform_int_values[0].el;
    param_2 = arr[x_66];
    param_3 = x_9.x_GLF_uniform_int_values[1].el;
    func_struct_S_i11_i1_(param_2, param_3);
    arr[x_66] = param_2;
  }
  if ((arr[x_9.x_GLF_uniform_int_values[0].el].data == x_9.x_GLF_uniform_int_values[1].el)) {
    float v = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_1 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_9.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[0].el));
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
