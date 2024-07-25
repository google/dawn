#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform m_block_std140_ubo {
  f16vec3 inner_0;
  f16vec3 inner_1;
  f16vec3 inner_2;
} m;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

f16mat3 load_m_inner() {
  return f16mat3(m.inner_0, m.inner_1, m.inner_2);
}

f16vec3 load_m_inner_p0(uint p0) {
  switch(p0) {
    case 0u: {
      return m.inner_0;
      break;
    }
    case 1u: {
      return m.inner_1;
      break;
    }
    case 2u: {
      return m.inner_2;
      break;
    }
    default: {
      return f16vec3(0.0hf);
      break;
    }
  }
}

void f() {
  f16mat3 p_m = load_m_inner();
  int tint_symbol = i();
  f16vec3 p_m_i = load_m_inner_p0(uint(tint_symbol));
  f16mat3 l_m = load_m_inner();
  f16vec3 l_m_i = load_m_inner_p0(uint(tint_symbol));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
