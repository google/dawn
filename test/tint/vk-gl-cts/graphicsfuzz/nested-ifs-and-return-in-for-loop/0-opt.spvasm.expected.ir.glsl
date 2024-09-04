SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
void main_1() {
  int i = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  {
    while(true) {
      if ((i < 10)) {
      } else {
        break;
      }
      x_GLF_color = vec4(1.0f);
      if ((1.0f > x_6.injectionSwitch.y)) {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        if (true) {
          return;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
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
