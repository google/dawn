#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  float a = vec2(1.0f).y;
  float b = vec3(1.0f).z;
  float c = vec4(1.0f).w;
}

