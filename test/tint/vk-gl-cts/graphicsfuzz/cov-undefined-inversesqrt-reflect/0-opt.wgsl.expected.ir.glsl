SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct buf1 {
  vec2 v1;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 m24 = mat2(vec2(0.0f), vec2(0.0f));
  float a = 0.0f;
  vec2 v2 = vec2(0.0f);
  vec2 v3 = vec2(0.0f);
  float x_40 = x_6.x_GLF_uniform_float_values[0].el;
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  float x_44 = x_8.v1.x;
  float x_47 = x_6.x_GLF_uniform_float_values[0].el;
  vec2 v = vec2(x_40, x_42);
  m24 = mat2(v, vec2((x_44 * 1.0f), x_47));
  mat2 x_51 = m24;
  a = x_51[0u][0u];
  v2 = vec2(1.0f);
  vec2 x_53 = v2;
  float x_54 = a;
  vec2 x_55 = vec2(x_54, 1.0f);
  v3 = reflect(x_53, x_55);
  float x_58 = x_6.x_GLF_uniform_float_values[0].el;
  vec2 x_59 = v3;
  float x_61 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4(x_58, x_59[0u], x_59[1u], x_61);
  float x_66 = x_8.v1.y;
  float x_68 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_66 == x_68)) {
    vec4 x_73 = x_GLF_color;
    x_GLF_color = vec4(x_73[0u], 0.0f, 0.0f, x_73[3u]);
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
