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


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float x_30 = 0.0f;
  float x_32 = 0.0f;
  x_32 = 0.0f;
  {
    while(true) {
      float x_33 = 0.0f;
      x_33 = x_32;
      if ((x_5.injectionSwitch.x < x_5.injectionSwitch.y)) {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        return;
      } else {
        {
          x_32 = x_33;
        }
        continue;
      }
      /* unreachable */
    }
  }
  /* unreachable */
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
