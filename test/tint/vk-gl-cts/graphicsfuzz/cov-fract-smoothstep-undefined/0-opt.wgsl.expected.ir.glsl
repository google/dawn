SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v1 = vec2(0.0f);
  vec2 b = vec2(0.0f);
  float a = 0.0f;
  bool x_51 = false;
  bool x_52_phi = false;
  float x_30 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec2(x_30, x_30);
  vec2 x_32 = v1;
  b = fract(x_32);
  vec2 x_34 = b;
  a = smoothstep(vec2(1.0f), vec2(2.0f), x_34)[0u];
  float x_38 = x_6.x_GLF_uniform_float_values[0].el;
  float x_39 = a;
  float x_40 = a;
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4(x_38, x_39, x_40, x_42);
  float x_45 = b.x;
  bool x_46 = (x_45 < 1.0f);
  x_52_phi = x_46;
  if (x_46) {
    float x_50 = b.y;
    x_51 = (x_50 < 1.0f);
    x_52_phi = x_51;
  }
  bool x_52 = x_52_phi;
  if (x_52) {
    float x_57 = x_6.x_GLF_uniform_float_values[0].el;
    float x_59 = b.x;
    float x_61 = b.y;
    float x_63 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_57, x_59, x_61, x_63);
  } else {
    float x_66 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_66, x_66, x_66, x_66);
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
