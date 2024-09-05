#version 310 es


struct Light {
  vec3 position;
  vec3 colour;
};

layout(binding = 1, std430)
buffer Lights_1_ssbo {
  Light light[];
} lights;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
