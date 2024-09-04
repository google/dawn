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
  int x_9[1] = int[1](0);
  int x_10 = 0;
  int x_6 = x_9[0u];
  {
    while(true) {
      x_GLF_color = vec4(0.0f);
      if ((x_9[x_5.zero] == x_6)) {
        x_10 = 1;
        break;
      }
      x_10 = 2;
      break;
    }
  }
  if (((x_10 == 1) | (x_10 == 2))) {
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
