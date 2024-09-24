#version 310 es


struct g {
  uvec3 a;
};

struct h {
  uint a;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  g tint_symbol_1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  h tint_symbol_3;
} v_1;
uint tint_int_dot(uvec3 x, uvec3 y) {
  return (((x.x * y.x) + (x.y * y.y)) + (x.z * y.z));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint l = tint_int_dot(v.tint_symbol_1.a, v.tint_symbol_1.a);
  v_1.tint_symbol_3.a = v.tint_symbol_1.a.x;
}
