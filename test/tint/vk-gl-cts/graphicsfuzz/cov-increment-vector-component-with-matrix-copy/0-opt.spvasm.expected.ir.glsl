SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  vec4 v = vec4(0.0f);
  mat3x4 m = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  mat4 indexable = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  a = x_6.x_GLF_uniform_int_values[0].el;
  v = vec4(x_9.x_GLF_uniform_float_values[2].el);
  float x_49 = x_9.x_GLF_uniform_float_values[3].el;
  vec4 v_1 = vec4(x_49, 0.0f, 0.0f, 0.0f);
  vec4 v_2 = vec4(0.0f, x_49, 0.0f, 0.0f);
  m = mat3x4(v_1, v_2, vec4(0.0f, 0.0f, x_49, 0.0f));
  int x_54 = a;
  int x_55 = a;
  m[x_54][x_55] = x_9.x_GLF_uniform_float_values[0].el;
  int x_59 = a;
  int x_78 = a;
  int x_79 = a;
  vec4 v_3 = vec4(m[0u].x, m[0u].y, m[0u].z, m[0u].w);
  vec4 v_4 = vec4(m[1u].x, m[1u].y, m[1u].z, m[1u].w);
  indexable = mat4(v_3, v_4, vec4(m[2u].x, m[2u].y, m[2u].z, m[2u].w), vec4(0.0f, 0.0f, 0.0f, 1.0f));
  v[x_59] = (v[x_59] + indexable[x_78][x_79]);
  if ((v.y == x_9.x_GLF_uniform_float_values[1].el)) {
    float v_5 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_6 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_7 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
