SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct buf2 {
  vec2 injectionSwitch;
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
uniform buf2 x_9;
uniform buf1 x_11;
bool continue_execution = true;
void main_1() {
  int a = 0;
  int i = 0;
  a = 1;
  int x_38 = x_6.x_GLF_uniform_int_values[0].el;
  int x_41 = x_6.x_GLF_uniform_int_values[1].el;
  int x_44 = x_6.x_GLF_uniform_int_values[1].el;
  int x_47 = x_6.x_GLF_uniform_int_values[0].el;
  float v = float(x_38);
  float v_1 = float(x_41);
  float v_2 = float(x_44);
  x_GLF_color = vec4(v, v_1, v_2, float(x_47));
  int x_51 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_51;
  {
    while(true) {
      int x_56 = i;
      int x_58 = x_6.x_GLF_uniform_int_values[2].el;
      if ((x_56 < x_58)) {
      } else {
        break;
      }
      int x_61 = a;
      a = (x_61 + 1);
      if ((x_61 > 3)) {
        break;
      }
      float x_67 = x_9.injectionSwitch.x;
      float x_69 = x_11.x_GLF_uniform_float_values[0].el;
      if ((x_67 > x_69)) {
        continue_execution = false;
      }
      {
        int x_73 = i;
        i = (x_73 + 1);
      }
      continue;
    }
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
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
