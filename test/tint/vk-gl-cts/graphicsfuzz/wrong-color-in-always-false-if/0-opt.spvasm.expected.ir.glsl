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


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float height = 0.0f;
  height = 256.0f;
  if ((x_6.injectionSwitch.y < 0.0f)) {
    x_GLF_color = mix(vec4(30.18000030517578125f, 8840.0f, 469.970001220703125f, 18.2399997711181640625f), vec4(9.8999996185302734375f, 0.10000000149011611938f, 1169.5386962890625f, 55.79000091552734375f), vec4(7612.9453125f, 797.010986328125f, height, 9.0f));
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
