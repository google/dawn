#version 310 es


struct Light {
  vec3 position;
  uint tint_pad_0;
  vec3 colour;
  uint tint_pad_1;
};

layout(binding = 0, std430)
buffer Lights_1_ssbo {
  Light light[];
} lights;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v = (uint(lights.light.length()) - 1u);
  uint v_1 = min(uint(0), v);
}
