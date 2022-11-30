#version 310 es

struct Inner {
  mat3 m;
  uint pad;
  uint pad_1;
  uint pad_2;
  uint pad_3;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140) uniform a_block_ubo {
  Outer inner[4];
} a;

void f() {
  Outer l_a[4] = a.inner;
  Outer l_a_3 = a.inner[3];
  Inner l_a_3_a[4] = a.inner[3].a;
  Inner l_a_3_a_2 = a.inner[3].a[2];
  mat3 l_a_3_a_2_m = a.inner[3].a[2].m;
  vec3 l_a_3_a_2_m_1 = a.inner[3].a[2].m[1];
  float l_a_3_a_2_m_1_0 = a.inner[3].a[2].m[1][0];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
