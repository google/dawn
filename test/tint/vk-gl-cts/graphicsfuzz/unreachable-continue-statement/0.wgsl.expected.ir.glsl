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
uniform buf0 x_7;
vec3 computeColor_() {
  int x_injected_loop_counter = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  x_injected_loop_counter = 1;
  {
    while(true) {
      float x_38 = x_7.injectionSwitch.x;
      if ((x_38 > 1.0f)) {
        float x_43 = x_7.injectionSwitch.x;
        if ((x_43 > 1.0f)) {
          {
          }
          continue;
        } else {
          {
          }
          continue;
        }
      }
      return vec3(1.0f);
    }
  }
  /* unreachable */
}
void main_1() {
  vec3 x_31 = computeColor_();
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
