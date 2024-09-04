SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injected;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
void main_1() {
  int m = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  if ((x_5.injected.x > x_5.injected.y)) {
    {
      while(true) {
        {
          if (true) { break; }
        }
        continue;
      }
    }
    m = 1;
    {
      while(true) {
        if (true) {
        } else {
          break;
        }
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        {
        }
        continue;
      }
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
