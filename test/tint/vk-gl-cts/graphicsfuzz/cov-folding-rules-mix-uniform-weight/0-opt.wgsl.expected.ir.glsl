SKIP: FAILED

#version 310 es

struct buf0 {
  float one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec3 v = vec3(0.0f);
  float d = 0.0f;
  float x_36 = x_6.one;
  v = mix(vec3(5.0f, 8.0f, -12.19999980926513671875f), vec3(1.0f, 4.90000009536743164062f, -2.09999990463256835938f), vec3(x_36, x_36, x_36));
  vec3 x_39 = v;
  d = distance(x_39, vec3(1.0f, 4.90000009536743164062f, -2.09999990463256835938f));
  float x_41 = d;
  if ((x_41 < 0.10000000149011611938f)) {
    float x_47 = v.x;
    x_GLF_color = vec4(x_47, 0.0f, 0.0f, 1.0f);
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
