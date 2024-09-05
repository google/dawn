#version 310 es


struct UniformBuffer {
  ivec3 d;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  UniformBuffer tint_symbol_1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 temp = (v.tint_symbol_1.d << (uvec3(0u) & uvec3(31u)));
}
