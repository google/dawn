SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[16];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int r[15] = int[15](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  int data[15] = int[15](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  int x_46 = x_6.x_GLF_uniform_int_values[0].el;
  r[x_46] = x_6.x_GLF_uniform_int_values[0].el;
  int x_51 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_51] = x_6.x_GLF_uniform_int_values[1].el;
  int x_56 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_56] = x_6.x_GLF_uniform_int_values[2].el;
  int x_61 = x_6.x_GLF_uniform_int_values[3].el;
  r[x_61] = x_6.x_GLF_uniform_int_values[3].el;
  int x_66 = x_6.x_GLF_uniform_int_values[4].el;
  r[x_66] = x_6.x_GLF_uniform_int_values[4].el;
  int x_71 = x_6.x_GLF_uniform_int_values[5].el;
  r[x_71] = -(x_6.x_GLF_uniform_int_values[1].el);
  int x_77 = x_6.x_GLF_uniform_int_values[8].el;
  r[x_77] = -(x_6.x_GLF_uniform_int_values[1].el);
  int x_83 = x_6.x_GLF_uniform_int_values[9].el;
  r[x_83] = -(x_6.x_GLF_uniform_int_values[1].el);
  int x_89 = x_6.x_GLF_uniform_int_values[10].el;
  r[x_89] = -(x_6.x_GLF_uniform_int_values[1].el);
  int x_95 = x_6.x_GLF_uniform_int_values[11].el;
  r[x_95] = -(x_6.x_GLF_uniform_int_values[1].el);
  int x_101 = x_6.x_GLF_uniform_int_values[6].el;
  r[x_101] = -(x_6.x_GLF_uniform_int_values[2].el);
  int x_107 = x_6.x_GLF_uniform_int_values[12].el;
  r[x_107] = -(x_6.x_GLF_uniform_int_values[2].el);
  int x_113 = x_6.x_GLF_uniform_int_values[13].el;
  r[x_113] = -(x_6.x_GLF_uniform_int_values[2].el);
  int x_119 = x_6.x_GLF_uniform_int_values[14].el;
  r[x_119] = -(x_6.x_GLF_uniform_int_values[2].el);
  int x_125 = x_6.x_GLF_uniform_int_values[15].el;
  r[x_125] = -(x_6.x_GLF_uniform_int_values[2].el);
  i = 0;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[5].el)) {
      } else {
        break;
      }
      int x_139 = i;
      int v = x_6.x_GLF_uniform_int_values[1].el;
      data[x_139] = ~(min(max(~(i), ~(i)), v));
      {
        i = (i + 1);
      }
      continue;
    }
  }
  i_1 = x_6.x_GLF_uniform_int_values[5].el;
  {
    while(true) {
      if ((i_1 < x_6.x_GLF_uniform_int_values[6].el)) {
      } else {
        break;
      }
      int x_162 = i_1;
      data[x_162] = ~(min(max(~(i_1), 0), 1));
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  i_2 = x_6.x_GLF_uniform_int_values[6].el;
  {
    while(true) {
      if ((i_2 < x_6.x_GLF_uniform_int_values[7].el)) {
      } else {
        break;
      }
      int x_181 = i_2;
      data[x_181] = ~(min(max(i_2, 0), 1));
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  i_3 = x_6.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i_3 < x_6.x_GLF_uniform_int_values[7].el)) {
      } else {
        break;
      }
      if ((data[i_3] != r[i_3])) {
        x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[0].el));
        return;
      }
      {
        i_3 = (i_3 + 1);
      }
      continue;
    }
  }
  float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_3 = float(x_6.x_GLF_uniform_int_values[0].el);
  x_GLF_color = vec4(v_1, v_2, v_3, float(x_6.x_GLF_uniform_int_values[1].el));
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
