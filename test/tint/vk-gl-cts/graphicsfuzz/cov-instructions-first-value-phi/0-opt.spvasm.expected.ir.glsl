SKIP: FAILED

#version 310 es

struct buf1 {
  vec2 v1;
};

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct S {
  int data;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_8;
uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void func_struct_S_i11_(inout S s) {
  if ((x_8.v1.x > x_8.v1.y)) {
    return;
  }
  s.data = x_10.x_GLF_uniform_int_values[0].el;
}
void main_1() {
  int i = 0;
  S arr[3] = S[3](S(0), S(0), S(0));
  int i_1 = 0;
  S param = S(0);
  int j = 0;
  bool x_132 = false;
  bool x_133 = false;
  bool x_142 = false;
  bool x_143 = false;
  i = x_10.x_GLF_uniform_int_values[2].el;
  {
    while(true) {
      if ((i < x_10.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      int x_56 = i;
      arr[x_56].data = i;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  i_1 = x_10.x_GLF_uniform_int_values[2].el;
  {
    while(true) {
      if ((i_1 < x_10.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if ((x_8.v1.x > x_8.v1.y)) {
        break;
      }
      if ((arr[i_1].data == x_10.x_GLF_uniform_int_values[3].el)) {
        int x_88 = i_1;
        param = arr[i_1];
        func_struct_S_i11_(param);
        arr[x_88] = param;
      } else {
        j = x_10.x_GLF_uniform_int_values[2].el;
        {
          while(true) {
            if ((j < x_10.x_GLF_uniform_int_values[0].el)) {
            } else {
              break;
            }
            if ((arr[j].data > x_10.x_GLF_uniform_int_values[4].el)) {
              continue_execution = false;
            }
            {
              j = (j + 1);
            }
            continue;
          }
        }
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  bool x_123 = (arr[x_10.x_GLF_uniform_int_values[2].el].data == x_10.x_GLF_uniform_int_values[2].el);
  x_133 = x_123;
  if (x_123) {
    x_132 = (arr[x_10.x_GLF_uniform_int_values[3].el].data == x_10.x_GLF_uniform_int_values[0].el);
    x_133 = x_132;
  }
  x_143 = x_133;
  if (x_133) {
    x_142 = (arr[x_10.x_GLF_uniform_int_values[1].el].data == x_10.x_GLF_uniform_int_values[1].el);
    x_143 = x_142;
  }
  if (x_143) {
    float v = float(x_10.x_GLF_uniform_int_values[3].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[2].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[2].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[3].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[2].el));
  }
}
main_out main() {
  main_1();
  main_out v_3 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
