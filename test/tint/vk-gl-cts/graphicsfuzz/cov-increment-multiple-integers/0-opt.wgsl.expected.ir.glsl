SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int b = 0;
  int c = 0;
  bool x_76 = false;
  bool x_83 = false;
  bool x_77_phi = false;
  bool x_84_phi = false;
  int x_31 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_31;
  int x_33 = x_6.x_GLF_uniform_int_values[2].el;
  b = x_33;
  c = 1;
  {
    while(true) {
      int x_38 = b;
      int x_40 = x_6.x_GLF_uniform_int_values[4].el;
      int x_42 = a;
      if (((x_38 < x_40) & (x_42 < 10))) {
      } else {
        break;
      }
      int x_46 = c;
      if ((x_46 > 5)) {
        break;
      }
      int x_50 = a;
      a = (x_50 + 1);
      int x_52 = c;
      c = (x_52 + 1);
      int x_54 = b;
      b = (x_54 + 1);
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_60 = a;
      int x_62 = x_6.x_GLF_uniform_int_values[1].el;
      if ((x_60 < x_62)) {
      } else {
        break;
      }
      {
        int x_65 = a;
        a = (x_65 + 1);
      }
      continue;
    }
  }
  int x_67 = a;
  int x_69 = x_6.x_GLF_uniform_int_values[1].el;
  bool x_70 = (x_67 == x_69);
  x_77_phi = x_70;
  if (x_70) {
    int x_73 = b;
    int x_75 = x_6.x_GLF_uniform_int_values[3].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  bool x_77 = x_77_phi;
  x_84_phi = x_77;
  if (x_77) {
    int x_80 = c;
    int x_82 = x_6.x_GLF_uniform_int_values[3].el;
    x_83 = (x_80 == x_82);
    x_84_phi = x_83;
  }
  bool x_84 = x_84_phi;
  if (x_84) {
    int x_89 = x_6.x_GLF_uniform_int_values[2].el;
    int x_92 = x_6.x_GLF_uniform_int_values[0].el;
    int x_95 = x_6.x_GLF_uniform_int_values[0].el;
    int x_98 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_89);
    float v_1 = float(x_92);
    float v_2 = float(x_95);
    x_GLF_color = vec4(v, v_1, v_2, float(x_98));
  } else {
    int x_102 = x_6.x_GLF_uniform_int_values[0].el;
    float x_103 = float(x_102);
    x_GLF_color = vec4(x_103, x_103, x_103, x_103);
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
