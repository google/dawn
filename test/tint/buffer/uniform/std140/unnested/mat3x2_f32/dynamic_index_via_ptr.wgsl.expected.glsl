#version 310 es

layout(binding = 0, std140) uniform m_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
} m;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

mat3x2 load_m_inner() {
  return mat3x2(m.inner_0, m.inner_1, m.inner_2);
}

vec2 load_m_inner_p0(uint p0) {
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
      return vec2(0.0f);
      break;
    }
  }
}

void f() {
  mat3x2 p_m = load_m_inner();
  int tint_symbol = i();
  vec2 p_m_i = load_m_inner_p0(uint(tint_symbol));
  mat3x2 l_m = load_m_inner();
  vec2 l_m_i = load_m_inner_p0(uint(tint_symbol));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
