#version 310 es

struct Light {
  vec3 position;
  vec3 colour;
};

struct Lights {
  Light light[];
};

Lights lights;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
