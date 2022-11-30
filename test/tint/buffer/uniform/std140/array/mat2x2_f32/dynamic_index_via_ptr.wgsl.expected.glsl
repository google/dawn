#version 310 es

struct mat2x2_f32 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat2x2_f32 inner[4];
} a;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

mat2 conv_mat2x2_f32(mat2x2_f32 val) {
  return mat2(val.col0, val.col1);
}

mat2[4] conv_arr4_mat2x2_f32(mat2x2_f32 val[4]) {
  mat2 arr[4] = mat2[4](mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x2_f32(val[i]);
    }
  }
  return arr;
}

vec2 load_a_inner_p0_p1(uint p0, uint p1) {
  switch(p1) {
    case 0u: {
      return a.inner[p0].col0;
      break;
    }
    case 1u: {
      return a.inner[p0].col1;
      break;
    }
    default: {
      return vec2(0.0f);
      break;
    }
  }
}

void f() {
  mat2 p_a[4] = conv_arr4_mat2x2_f32(a.inner);
  int tint_symbol = i();
  mat2 p_a_i = conv_mat2x2_f32(a.inner[tint_symbol]);
  int tint_symbol_1 = i();
  vec2 p_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
  mat2 l_a[4] = conv_arr4_mat2x2_f32(a.inner);
  mat2 l_a_i = conv_mat2x2_f32(a.inner[tint_symbol]);
  vec2 l_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
