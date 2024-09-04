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
uniform buf0 x_5;
void main_1() {
  bool GLF_live12c5 = false;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  {
    while(true) {
      if ((x_5.injectionSwitch.y < 0.0f)) {
        GLF_live12c5 = false;
        if (GLF_live12c5) {
          {
            if (true) { break; }
          }
          continue;
        } else {
          {
            if (true) { break; }
          }
          continue;
        }
      }
      break;
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
