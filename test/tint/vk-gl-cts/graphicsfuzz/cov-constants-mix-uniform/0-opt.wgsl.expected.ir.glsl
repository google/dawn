SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 mixed = vec2(0.0f);
  vec2 x_30 = x_6.one;
  mixed = mix(vec2(1.0f), x_30, vec2(0.5f));
  vec2 x_33 = mixed;
  if (all((x_33 == vec2(1.0f)))) {
    float x_40 = mixed.x;
    x_GLF_color = vec4(x_40, 0.0f, 0.0f, 1.0f);
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
