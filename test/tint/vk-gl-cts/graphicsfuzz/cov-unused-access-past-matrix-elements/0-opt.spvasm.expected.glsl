SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

layout(binding = 1) uniform buf1_1 {
  strided_arr x_GLF_uniform_float_values[3];
} x_6;

layout(binding = 0) uniform buf0_1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
} x_8;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  mat4x3 m43 = mat4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  strided_arr sums[3] = strided_arr[3](strided_arr(0.0f), strided_arr(0.0f), strided_arr(0.0f));
  int i = 0;
  int a = 0;
  int x_67_phi = 0;
  float x_44 = x_6.x_GLF_uniform_float_values[1].el;
  vec3 x_48 = vec3(0.0f);
  m43 = mat4x3(vec3(x_44, 0.0f, 0.0f), vec3(0.0f, x_44, 0.0f), vec3(0.0f, 0.0f, x_44), vec3(0.0f));
  int x_51 = x_8.x_GLF_uniform_int_values[0].el;
  int x_53 = x_8.x_GLF_uniform_int_values[0].el;
  float x_55 = x_6.x_GLF_uniform_float_values[0].el;
  m43[x_51][x_53] = x_55;
  float x_58 = x_6.x_GLF_uniform_float_values[0].el;
  float x_60 = x_6.x_GLF_uniform_float_values[0].el;
  float x_62 = x_6.x_GLF_uniform_float_values[0].el;
  strided_arr tint_symbol_1 = strided_arr(x_58);
  strided_arr tint_symbol_2 = strided_arr(x_60);
  strided_arr tint_symbol_3 = strided_arr(x_62);
  strided_arr tint_symbol_4[3] = strided_arr[3](tint_symbol_1, tint_symbol_2, tint_symbol_3);
  sums = tint_symbol_4;
  int x_65 = x_8.x_GLF_uniform_int_values[0].el;
  i = x_65;
  x_67_phi = x_65;
  while (true) {
    int x_67 = x_67_phi;
    int x_73 = x_8.x_GLF_uniform_int_values[3].el;
    if ((x_67 < x_73)) {
    } else {
      break;
    }
    int x_77 = x_8.x_GLF_uniform_int_values[0].el;
    int x_79 = x_8.x_GLF_uniform_int_values[0].el;
    float x_81 = m43[x_67][x_79];
    float x_83 = sums[x_77].el;
    sums[x_77].el = (x_83 + x_81);
    {
      int x_68 = (x_67 + 1);
      i = x_68;
      x_67_phi = x_68;
    }
  }
  int x_87 = x_8.x_GLF_uniform_int_values[1].el;
  if ((x_87 == 1)) {
    a = 4;
    int x_92 = x_8.x_GLF_uniform_int_values[2].el;
    int x_94 = x_8.x_GLF_uniform_int_values[0].el;
    float x_96 = m43[4][x_94];
    float x_98 = sums[x_92].el;
    sums[x_92].el = (x_98 + x_96);
  }
  int x_102 = x_8.x_GLF_uniform_int_values[1].el;
  float x_104 = sums[x_102].el;
  int x_106 = x_8.x_GLF_uniform_int_values[0].el;
  float x_108 = sums[x_106].el;
  float x_111 = x_6.x_GLF_uniform_float_values[2].el;
  if (((x_104 + x_108) == x_111)) {
    int x_117 = x_8.x_GLF_uniform_int_values[0].el;
    int x_120 = x_8.x_GLF_uniform_int_values[1].el;
    int x_123 = x_8.x_GLF_uniform_int_values[1].el;
    int x_126 = x_8.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_117), float(x_120), float(x_123), float(x_126));
  } else {
    int x_130 = x_8.x_GLF_uniform_int_values[1].el;
    float x_131 = float(x_130);
    x_GLF_color = vec4(x_131, x_131, x_131, x_131);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_5 = main_out(x_GLF_color);
  return tint_symbol_5;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
Error parsing GLSL shader:
ERROR: 0:77: '[' :  matrix index out of range '4'
ERROR: 0:77: '=' :  cannot convert from ' temp mediump 3-component vector of float' to ' temp mediump float'
ERROR: 0:77: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



