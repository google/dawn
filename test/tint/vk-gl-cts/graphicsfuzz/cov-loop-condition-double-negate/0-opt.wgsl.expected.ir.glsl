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
  bool x_86 = false;
  bool x_77_phi = false;
  bool x_87_phi = false;
  int x_33 = x_6.x_GLF_uniform_int_values[3].el;
  int x_35 = x_6.x_GLF_uniform_int_values[5].el;
  int x_37 = x_6.x_GLF_uniform_int_values[2].el;
  arr = int[3](x_33, x_35, x_37);
  index = 1;
  {
    while(true) {
      bool x_51 = false;
      bool x_52_phi = false;
      x_52_phi = true;
      if (true) {
        int x_46 = x_6.x_GLF_uniform_int_values[0].el;
        int x_48 = index;
        x_51 = !(((x_46 == 1) & (x_48 <= 1)));
        x_52_phi = x_51;
      }
      bool x_52 = x_52_phi;
      if (!(x_52)) {
      } else {
        break;
      }
      int x_55 = index;
      int x_56_save = x_55;
      int x_57 = arr[x_56_save];
      arr[x_56_save] = (x_57 + 1);
      int x_59 = index;
      index = (x_59 + 1);
      {
      }
      continue;
    }
  }
  int x_62 = x_6.x_GLF_uniform_int_values[1].el;
  int x_64 = arr[x_62];
  int x_66 = x_6.x_GLF_uniform_int_values[3].el;
  bool x_67 = (x_64 == x_66);
  x_77_phi = x_67;
  if (x_67) {
    int x_71 = x_6.x_GLF_uniform_int_values[0].el;
    int x_73 = arr[x_71];
    int x_75 = x_6.x_GLF_uniform_int_values[4].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  bool x_77 = x_77_phi;
  x_87_phi = x_77;
  if (x_77) {
    int x_81 = x_6.x_GLF_uniform_int_values[3].el;
    int x_83 = arr[x_81];
    int x_85 = x_6.x_GLF_uniform_int_values[2].el;
    x_86 = (x_83 == x_85);
    x_87_phi = x_86;
  }
  bool x_87 = x_87_phi;
  if (x_87) {
    int x_92 = x_6.x_GLF_uniform_int_values[0].el;
    int x_95 = x_6.x_GLF_uniform_int_values[1].el;
    int x_98 = x_6.x_GLF_uniform_int_values[1].el;
    int x_101 = x_6.x_GLF_uniform_int_values[0].el;
    float v = float(x_92);
    float v_1 = float(x_95);
    float v_2 = float(x_98);
    x_GLF_color = vec4(v, v_1, v_2, float(x_101));
  } else {
    int x_105 = x_6.x_GLF_uniform_int_values[1].el;
    float x_106 = float(x_105);
    x_GLF_color = vec4(x_106, x_106, x_106, x_106);
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
