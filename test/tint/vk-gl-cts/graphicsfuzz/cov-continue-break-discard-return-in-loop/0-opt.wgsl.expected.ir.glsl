SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct buf1 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
uniform buf1 x_7;
bool continue_execution = true;
void main_1() {
  int x_28 = x_5.x_GLF_uniform_int_values[0].el;
  int x_31 = x_5.x_GLF_uniform_int_values[1].el;
  int x_34 = x_5.x_GLF_uniform_int_values[1].el;
  int x_37 = x_5.x_GLF_uniform_int_values[0].el;
  float v = float(x_28);
  float v_1 = float(x_31);
  float v_2 = float(x_34);
  x_GLF_color = vec4(v, v_1, v_2, float(x_37));
  {
    while(true) {
      int x_45 = x_7.zero;
      int x_47 = x_5.x_GLF_uniform_int_values[0].el;
      if ((x_45 == x_47)) {
        {
          if (true) { break; }
        }
        continue;
      }
      int x_52 = x_7.zero;
      int x_54 = x_5.x_GLF_uniform_int_values[2].el;
      if ((x_52 == x_54)) {
        break;
      }
      int x_59 = x_7.zero;
      int x_61 = x_5.x_GLF_uniform_int_values[3].el;
      if ((x_59 == x_61)) {
        continue_execution = false;
      }
      return;
    }
  }
  int x_66 = x_5.x_GLF_uniform_int_values[1].el;
  float x_67 = float(x_66);
  x_GLF_color = vec4(x_67, x_67, x_67, x_67);
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
ERROR: 0:16: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
