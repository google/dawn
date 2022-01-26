#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}

void main() {
  unused_entry_point();
}

struct Light {
  vec3 position;
  vec3 colour;
};

layout(binding = 1) buffer Lights_1 {
  Light light[];
} lights;
