#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2D tex;
void tint_symbol() {
  imageStore(tex, ivec2(0), vec4(0.0f));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
