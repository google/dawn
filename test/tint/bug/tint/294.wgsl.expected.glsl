#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct Light {
  vec3 position;
  uint pad;
  vec3 colour;
  uint pad_1;
};

layout(binding = 1, std430) buffer Lights_ssbo {
  Light light[];
} lights;

