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
  float x_166 = x_8.v1.x;
  float x_168 = x_8.v1.y;
  if ((x_166 > x_168)) {
    return;
  }
  int x_173 = x_10.x_GLF_uniform_int_values[0].el;
  s.data = x_173;
}
void main_1() {
  int i = 0;
  S arr[3] = S[3](S(0), S(0), S(0));
  int i_1 = 0;
  S param = S(0);
  int j = 0;
  bool x_132 = false;
  bool x_142 = false;
  bool x_133_phi = false;
  bool x_143_phi = false;
  int x_46 = x_10.x_GLF_uniform_int_values[2].el;
  i = x_46;
  {
    while(true) {
      int x_51 = i;
      int x_53 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_51 < x_53)) {
      } else {
        break;
      }
      int x_56 = i;
      int x_57 = i;
      arr[x_56].data = x_57;
      {
        int x_59 = i;
        i = (x_59 + 1);
      }
      continue;
    }
  }
  int x_62 = x_10.x_GLF_uniform_int_values[2].el;
  i_1 = x_62;
  {
    while(true) {
      int x_67 = i_1;
      int x_69 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_67 < x_69)) {
      } else {
        break;
      }
      float x_73 = x_8.v1.x;
      float x_75 = x_8.v1.y;
      if ((x_73 > x_75)) {
        break;
      }
      int x_79 = i_1;
      int x_81 = arr[x_79].data;
      int x_83 = x_10.x_GLF_uniform_int_values[3].el;
      if ((x_81 == x_83)) {
        int x_88 = i_1;
        S x_90 = arr[x_88];
        param = x_90;
        func_struct_S_i11_(param);
        S x_92 = param;
        arr[x_88] = x_92;
      } else {
        int x_95 = x_10.x_GLF_uniform_int_values[2].el;
        j = x_95;
        {
          while(true) {
            int x_100 = j;
            int x_102 = x_10.x_GLF_uniform_int_values[0].el;
            if ((x_100 < x_102)) {
            } else {
              break;
            }
            int x_105 = j;
            int x_107 = arr[x_105].data;
            int x_109 = x_10.x_GLF_uniform_int_values[4].el;
            if ((x_107 > x_109)) {
              continue_execution = false;
            }
            {
              int x_113 = j;
              j = (x_113 + 1);
            }
            continue;
          }
        }
      }
      {
        int x_115 = i_1;
        i_1 = (x_115 + 1);
      }
      continue;
    }
  }
  int x_118 = x_10.x_GLF_uniform_int_values[2].el;
  int x_120 = arr[x_118].data;
  int x_122 = x_10.x_GLF_uniform_int_values[2].el;
  bool x_123 = (x_120 == x_122);
  x_133_phi = x_123;
  if (x_123) {
    int x_127 = x_10.x_GLF_uniform_int_values[3].el;
    int x_129 = arr[x_127].data;
    int x_131 = x_10.x_GLF_uniform_int_values[0].el;
    x_132 = (x_129 == x_131);
    x_133_phi = x_132;
  }
  bool x_133 = x_133_phi;
  x_143_phi = x_133;
  if (x_133) {
    int x_137 = x_10.x_GLF_uniform_int_values[1].el;
    int x_139 = arr[x_137].data;
    int x_141 = x_10.x_GLF_uniform_int_values[1].el;
    x_142 = (x_139 == x_141);
    x_143_phi = x_142;
  }
  bool x_143 = x_143_phi;
  if (x_143) {
    int x_148 = x_10.x_GLF_uniform_int_values[3].el;
    int x_151 = x_10.x_GLF_uniform_int_values[2].el;
    int x_154 = x_10.x_GLF_uniform_int_values[2].el;
    int x_157 = x_10.x_GLF_uniform_int_values[3].el;
    float v = float(x_148);
    float v_1 = float(x_151);
    float v_2 = float(x_154);
    x_GLF_color = vec4(v, v_1, v_2, float(x_157));
  } else {
    int x_161 = x_10.x_GLF_uniform_int_values[2].el;
    float x_162 = float(x_161);
    x_GLF_color = vec4(x_162, x_162, x_162, x_162);
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
