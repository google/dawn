SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[6];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int arr[3] = int[3](0, 0, 0);
  int index = 0;
  bool x_76 = false;
  bool x_77 = false;
  bool x_86 = false;
  bool x_87 = false;
  arr = int[3](x_6.x_GLF_uniform_int_values[3].el, x_6.x_GLF_uniform_int_values[5].el, x_6.x_GLF_uniform_int_values[2].el);
  index = 1;
  {
    while(true) {
      bool x_51 = false;
      bool x_52 = false;
      x_52 = true;
      if (true) {
        x_51 = !(((x_6.x_GLF_uniform_int_values[0].el == 1) & (index <= 1)));
        x_52 = x_51;
      }
      if (!(x_52)) {
      } else {
        break;
      }
      int x_55 = index;
      int x_56_save = x_55;
      arr[x_56_save] = (arr[x_55] + 1);
      index = (index + 1);
      {
      }
      continue;
    }
  }
  bool x_67 = (arr[x_6.x_GLF_uniform_int_values[1].el] == x_6.x_GLF_uniform_int_values[3].el);
  x_77 = x_67;
  if (x_67) {
    x_76 = (arr[x_6.x_GLF_uniform_int_values[0].el] == x_6.x_GLF_uniform_int_values[4].el);
    x_77 = x_76;
  }
  x_87 = x_77;
  if (x_77) {
    x_86 = (arr[x_6.x_GLF_uniform_int_values[3].el] == x_6.x_GLF_uniform_int_values[2].el);
    x_87 = x_86;
  }
  if (x_87) {
    float v = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
