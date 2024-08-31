#version 310 es

struct UniformBuffer {
  ivec3 d;
};

uniform UniformBuffer u_input;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 temp = (u_input.d << (uvec3(0u) & uvec3(31u)));
}
