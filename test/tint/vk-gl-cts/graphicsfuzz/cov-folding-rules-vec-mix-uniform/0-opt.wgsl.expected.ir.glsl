SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 zeroOne;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v = vec2(0.0f);
  float d = 0.0f;
  vec2 x_37 = x_6.zeroOne;
  v = mix(vec2(2.0f, 3.0f), vec2(4.0f, 5.0f), x_37);
  vec2 x_39 = v;
  d = distance(x_39, vec2(2.0f, 5.0f));
  float x_41 = d;
  if ((x_41 < 0.10000000149011611938f)) {
    float x_47 = v.x;
    float x_50 = v.y;
    x_GLF_color = vec4((x_47 - 1.0f), (x_50 - 5.0f), 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
