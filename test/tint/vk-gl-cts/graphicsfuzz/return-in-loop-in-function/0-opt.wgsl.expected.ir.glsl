SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
float f_() {
  int i = 0;
  i = 1;
  {
    while(true) {
      int x_8 = i;
      if ((x_8 < 10)) {
      } else {
        break;
      }
      int x_9 = i;
      if ((float(x_9) >= 1.0f)) {
        return 1.0f;
      } else {
        {
          int x_10 = i;
          i = (x_10 + 1);
        }
        continue;
      }
      /* unreachable */
    }
  }
  return 1.0f;
}
void main_1() {
  vec4 c = vec4(0.0f);
  int i_1 = 0;
  c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  i_1 = 0;
  {
    while(true) {
      int x_12 = i_1;
      if ((x_12 < 1)) {
      } else {
        break;
      }
      {
        float x_39 = f_();
        c[0u] = x_39;
        int x_13 = i_1;
        i_1 = (x_13 + 1);
      }
      continue;
    }
  }
  vec4 x_41 = c;
  x_GLF_color = x_41;
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
