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
  int x_44 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_44;
  float x_46 = x_9.x_GLF_uniform_float_values[2].el;
  v = vec4(x_46, x_46, x_46, x_46);
  float x_49 = x_9.x_GLF_uniform_float_values[3].el;
  vec4 v_1 = vec4(x_49, 0.0f, 0.0f, 0.0f);
  vec4 v_2 = vec4(0.0f, x_49, 0.0f, 0.0f);
  m = mat3x4(v_1, v_2, vec4(0.0f, 0.0f, x_49, 0.0f));
  int x_54 = a;
  int x_55 = a;
  float x_57 = x_9.x_GLF_uniform_float_values[0].el;
  m[x_54][x_55] = x_57;
  int x_59 = a;
  mat3x4 x_60 = m;
  int x_78 = a;
  int x_79 = a;
  vec4 v_3 = vec4(x_60[0u][0u], x_60[0u][1u], x_60[0u][2u], x_60[0u][3u]);
  vec4 v_4 = vec4(x_60[1u][0u], x_60[1u][1u], x_60[1u][2u], x_60[1u][3u]);
  indexable = mat4(v_3, v_4, vec4(x_60[2u][0u], x_60[2u][1u], x_60[2u][2u], x_60[2u][3u]), vec4(0.0f, 0.0f, 0.0f, 1.0f));
  float x_81 = indexable[x_78][x_79];
  float x_83 = v[x_59];
  v[x_59] = (x_83 + x_81);
  float x_87 = v.y;
  float x_89 = x_9.x_GLF_uniform_float_values[1].el;
  if ((x_87 == x_89)) {
    int x_95 = x_6.x_GLF_uniform_int_values[0].el;
    int x_98 = x_6.x_GLF_uniform_int_values[1].el;
    int x_101 = x_6.x_GLF_uniform_int_values[1].el;
    int x_104 = x_6.x_GLF_uniform_int_values[0].el;
    float v_5 = float(x_95);
    float v_6 = float(x_98);
    float v_7 = float(x_101);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_104));
  } else {
    int x_108 = x_6.x_GLF_uniform_int_values[1].el;
    float x_109 = float(x_108);
    x_GLF_color = vec4(x_109, x_109, x_109, x_109);
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
