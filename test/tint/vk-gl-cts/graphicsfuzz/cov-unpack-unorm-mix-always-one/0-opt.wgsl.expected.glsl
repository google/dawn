vk-gl-cts/graphicsfuzz/cov-unpack-unorm-mix-always-one/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<u32, 1>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-unpack-unorm-mix-always-one/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 2>;
              ^^^^^^

vk-gl-cts/graphicsfuzz/cov-unpack-unorm-mix-always-one/0-opt.wgsl:13:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_2 = @stride(16) array<f32, 3>;
              ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  uint el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_uint_values[1];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
};

struct tint_padded_array_element_2 {
  float el;
};

struct buf2 {
  tint_padded_array_element_2 x_GLF_uniform_float_values[3];
};

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_uint_values[1];
} x_6;

layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
} x_8;

layout(binding = 2) uniform buf2_1 {
  tint_padded_array_element_2 x_GLF_uniform_float_values[3];
} x_10;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  uint x_39 = x_6.x_GLF_uniform_uint_values[0].el;
  uint x_41 = x_6.x_GLF_uniform_uint_values[0].el;
  v = unpackUnorm4x8((x_39 / (true ? 92382u : x_41)));
  vec4 x_45 = v;
  int x_47 = x_8.x_GLF_uniform_int_values[0].el;
  int x_50 = x_8.x_GLF_uniform_int_values[0].el;
  int x_53 = x_8.x_GLF_uniform_int_values[0].el;
  float x_56 = x_10.x_GLF_uniform_float_values[1].el;
  float x_58 = x_10.x_GLF_uniform_float_values[2].el;
  float x_63 = x_10.x_GLF_uniform_float_values[0].el;
  if ((distance(x_45, vec4(float(x_47), float(x_50), float(x_53), (x_56 / x_58))) < x_63)) {
    int x_69 = x_8.x_GLF_uniform_int_values[1].el;
    int x_72 = x_8.x_GLF_uniform_int_values[0].el;
    int x_75 = x_8.x_GLF_uniform_int_values[0].el;
    int x_78 = x_8.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_69), float(x_72), float(x_75), float(x_78));
  } else {
    int x_82 = x_8.x_GLF_uniform_int_values[0].el;
    float x_83 = float(x_82);
    x_GLF_color = vec4(x_83, x_83, x_83, x_83);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_GLF_color);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
