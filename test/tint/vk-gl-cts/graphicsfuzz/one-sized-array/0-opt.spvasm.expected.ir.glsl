SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
void main_1() {
  int x_10[1] = int[1](0);
  int x_9[1] = int[1](0);
  int x_7 = 0;
  int x_11 = 0;
  x_9[0] = x_5.zero;
  x_10 = x_9;
  x_7 = x_9[0];
  switch(0u) {
    default:
    {
      x_GLF_color = vec4(0.0f);
      if ((x_10[0] == x_7)) {
        x_11 = 1;
        break;
      }
      x_11 = 2;
      break;
    }
  }
  if ((x_11 == 1)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
