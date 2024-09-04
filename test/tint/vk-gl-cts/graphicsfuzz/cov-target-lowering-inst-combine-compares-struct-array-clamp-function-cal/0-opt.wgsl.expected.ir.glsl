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
  {
    while(true) {
      float x_174 = x_8.v1.x;
      float x_176 = x_8.v1.y;
      if ((x_174 > x_176)) {
      } else {
        break;
      }
      return;
    }
  }
  int x_180 = x_10.x_GLF_uniform_int_values[0].el;
  s.data = x_180;
}
void main_1() {
  int i = 0;
  S arr[3] = S[3](S(0), S(0), S(0));
  int i_1 = 0;
  S param = S(0);
  int j = 0;
  bool x_136 = false;
  bool x_146 = false;
  bool x_137_phi = false;
  bool x_147_phi = false;
  int x_46 = x_10.x_GLF_uniform_int_values[2].el;
  i = x_46;
  {
    while(true) {
      int x_51 = i;
      int x_53 = x_10.x_GLF_uniform_int_values[1].el;
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
      int x_69 = x_10.x_GLF_uniform_int_values[1].el;
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
      int x_83 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_81 == x_83)) {
        int x_88 = i_1;
        int x_91 = x_10.x_GLF_uniform_int_values[3].el;
        arr[min(max(x_88, 0), 3)].data = x_91;
        S x_94 = arr[2];
        param = x_94;
        func_struct_S_i11_(param);
        S x_96 = param;
        arr[2] = x_96;
      } else {
        int x_99 = x_10.x_GLF_uniform_int_values[2].el;
        j = x_99;
        {
          while(true) {
            int x_104 = j;
            int x_106 = x_10.x_GLF_uniform_int_values[1].el;
            if ((x_104 < x_106)) {
            } else {
              break;
            }
            int x_109 = j;
            int x_111 = arr[x_109].data;
            int x_113 = x_10.x_GLF_uniform_int_values[4].el;
            if ((x_111 > x_113)) {
              continue_execution = false;
            }
            {
              int x_117 = j;
              j = (x_117 + 1);
            }
            continue;
          }
        }
      }
      {
        int x_119 = i_1;
        i_1 = (x_119 + 1);
      }
      continue;
    }
  }
  int x_122 = x_10.x_GLF_uniform_int_values[2].el;
  int x_124 = arr[x_122].data;
  int x_126 = x_10.x_GLF_uniform_int_values[2].el;
  bool x_127 = (x_124 == x_126);
  x_137_phi = x_127;
  if (x_127) {
    int x_131 = x_10.x_GLF_uniform_int_values[0].el;
    int x_133 = arr[x_131].data;
    int x_135 = x_10.x_GLF_uniform_int_values[3].el;
    x_136 = (x_133 == x_135);
    x_137_phi = x_136;
  }
  bool x_137 = x_137_phi;
  x_147_phi = x_137;
  if (x_137) {
    int x_141 = x_10.x_GLF_uniform_int_values[3].el;
    int x_143 = arr[x_141].data;
    int x_145 = x_10.x_GLF_uniform_int_values[0].el;
    x_146 = (x_143 == x_145);
    x_147_phi = x_146;
  }
  bool x_147 = x_147_phi;
  if (x_147) {
    int x_152 = x_10.x_GLF_uniform_int_values[0].el;
    int x_155 = x_10.x_GLF_uniform_int_values[2].el;
    int x_158 = x_10.x_GLF_uniform_int_values[2].el;
    int x_161 = x_10.x_GLF_uniform_int_values[0].el;
    float v = float(x_152);
    float v_1 = float(x_155);
    float v_2 = float(x_158);
    x_GLF_color = vec4(v, v_1, v_2, float(x_161));
  } else {
    int x_165 = x_10.x_GLF_uniform_int_values[2].el;
    float x_166 = float(x_165);
    x_GLF_color = vec4(x_166, x_166, x_166, x_166);
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
