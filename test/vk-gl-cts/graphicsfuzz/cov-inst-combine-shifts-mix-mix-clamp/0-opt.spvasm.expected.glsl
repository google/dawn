SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_int_values[3];
};
struct tint_padded_array_element_1 {
  float el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[2];
};

layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_int_values[3];
} x_6;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[2];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int A[2] = int[2](0, 0);
  int i = 0;
  int a = 0;
  vec2 v1 = vec2(0.0f, 0.0f);
  vec2 v2 = vec2(0.0f, 0.0f);
  int b = 0;
  int x_46 = x_6.x_GLF_uniform_int_values[2].el;
  int x_48 = x_6.x_GLF_uniform_int_values[0].el;
  A[x_46] = x_48;
  int x_51 = x_6.x_GLF_uniform_int_values[0].el;
  int x_53 = x_6.x_GLF_uniform_int_values[1].el;
  A[x_51] = x_53;
  int x_56 = x_6.x_GLF_uniform_int_values[0].el;
  i = x_56;
  while (true) {
    int x_61 = i;
    int x_63 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_61 > x_63)) {
    } else {
      break;
    }
    i = (i - 1);
  }
  float x_69 = x_10.x_GLF_uniform_float_values[1].el;
  float x_71 = x_10.x_GLF_uniform_float_values[1].el;
  int x_76 = A[((x_69 >= x_71) ? 1 : i)];
  a = x_76;
  int x_78 = x_6.x_GLF_uniform_int_values[0].el;
  int x_80 = a;
  int x_84 = x_6.x_GLF_uniform_int_values[0].el;
  int x_87 = x_6.x_GLF_uniform_int_values[0].el;
  float x_91 = x_10.x_GLF_uniform_float_values[1].el;
  float x_93 = x_10.x_GLF_uniform_float_values[0].el;
  v1 = (bvec2((x_91 < x_93), true) ? vec2(float(x_84), float(x_87)) : vec2(float(x_78), float(x_80)));
  int x_98 = x_6.x_GLF_uniform_int_values[2].el;
  float x_100 = v1[x_98];
  int x_103 = x_6.x_GLF_uniform_int_values[0].el;
  float x_105 = v1[x_103];
  v2 = (bvec2(false, false) ? vec2(x_105, x_105) : vec2(x_100, x_100));
  int x_109 = x_6.x_GLF_uniform_int_values[1].el;
  float x_110 = float(x_109);
  int x_113 = x_6.x_GLF_uniform_int_values[0].el;
  float x_114 = float(x_113);
  int x_121 = A[int(clamp(vec2(x_110, x_110), vec2(x_114, x_114), v2).x)];
  b = x_121;
  int x_122 = b;
  int x_124 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_122 == x_124)) {
    int x_130 = x_6.x_GLF_uniform_int_values[0].el;
    int x_133 = x_6.x_GLF_uniform_int_values[2].el;
    int x_136 = x_6.x_GLF_uniform_int_values[2].el;
    int x_139 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_130), float(x_133), float(x_136), float(x_139));
  } else {
    int x_143 = x_6.x_GLF_uniform_int_values[2].el;
    float x_144 = float(x_143);
    x_GLF_color = vec4(x_144, x_144, x_144, x_144);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:59: '' : boolean expression expected 
ERROR: 0:59: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



