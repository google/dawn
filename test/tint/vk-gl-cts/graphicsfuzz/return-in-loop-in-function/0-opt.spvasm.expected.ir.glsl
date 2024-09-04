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
      if ((i < 10)) {
      } else {
        break;
      }
      if ((float(i) >= 1.0f)) {
        return 1.0f;
      } else {
        {
          i = (i + 1);
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
      if ((i_1 < 1)) {
      } else {
        break;
      }
      {
        float x_39 = f_();
        c[0u] = x_39;
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  x_GLF_color = c;
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
