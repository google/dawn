#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 a = ivec3(1, 2, 3);
  ivec3 b = ivec3(4, 5, 6);
  uvec3 v = uvec3(a);
  ivec3 r = ivec3((v + uvec3(b)));
}
