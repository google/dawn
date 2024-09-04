SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  int a = 0;
  a = 1;
  {
    while(true) {
      int x_29 = a;
      int x_31 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_29 >= x_31)) {
        break;
      }
      if (true) {
        continue_execution = false;
      }
      int x_37 = a;
      a = (x_37 + 1);
      {
        int x_39 = a;
        if (!((x_39 != 1))) { break; }
      }
      continue;
    }
  }
  int x_41 = a;
  if ((x_41 == 1)) {
    int x_47 = x_6.x_GLF_uniform_int_values[0].el;
    int x_50 = x_6.x_GLF_uniform_int_values[0].el;
    int x_53 = x_6.x_GLF_uniform_int_values[1].el;
    float v = float(x_47);
    float v_1 = float(x_50);
    x_GLF_color = vec4(1.0f, v, v_1, float(x_53));
  } else {
    int x_57 = x_6.x_GLF_uniform_int_values[0].el;
    float x_58 = float(x_57);
    x_GLF_color = vec4(x_58, x_58, x_58, x_58);
  }
}
main_out main() {
  main_1();
  main_out v_2 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_2;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
