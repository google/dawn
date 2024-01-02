#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
  f16vec3 col2;
  f16vec3 col3;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat4x3_f16 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

f16mat4x3 conv_mat4x3_f16(mat4x3_f16 val) {
  return f16mat4x3(val.col0, val.col1, val.col2, val.col3);
}

f16mat4x3[4] conv_arr4_mat4x3_f16(mat4x3_f16 val[4]) {
  f16mat4x3 arr[4] = f16mat4x3[4](f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x3_f16(val[i]);
    }
  }
  return arr;
}

f16vec3 load_a_inner_p0_p1(uint p0, uint p1) {
  switch(p1) {
    case 0u: {
      return a.inner[p0].col0;
      break;
    }
    case 1u: {
      return a.inner[p0].col1;
      break;
    }
    case 2u: {
      return a.inner[p0].col2;
      break;
    }
    case 3u: {
      return a.inner[p0].col3;
      break;
    }
    default: {
      return f16vec3(0.0hf);
      break;
    }
  }
}

float16_t load_a_inner_p0_p1_0(uint p0, uint p1) {
  switch(p1) {
    case 0u: {
      return a.inner[p0].col0[0u];
      break;
    }
    case 1u: {
      return a.inner[p0].col1[0u];
      break;
    }
    case 2u: {
      return a.inner[p0].col2[0u];
      break;
    }
    case 3u: {
      return a.inner[p0].col3[0u];
      break;
    }
    default: {
      return 0.0hf;
      break;
    }
  }
}

void f() {
  f16mat4x3 p_a[4] = conv_arr4_mat4x3_f16(a.inner);
  int tint_symbol = i();
  f16mat4x3 p_a_i = conv_mat4x3_f16(a.inner[tint_symbol]);
  int tint_symbol_1 = i();
  f16vec3 p_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
  f16mat4x3 l_a[4] = conv_arr4_mat4x3_f16(a.inner);
  f16mat4x3 l_a_i = conv_mat4x3_f16(a.inner[tint_symbol]);
  f16vec3 l_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
  s.inner = (((load_a_inner_p0_p1_0(uint(tint_symbol), uint(tint_symbol_1)) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
