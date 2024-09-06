#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec3 v() {
  return vec3(0.0f);
}

void f() {
  vec3 a = vec3(1.0f);
  vec3 b = vec3(a);
  vec3 tint_symbol = v();
  vec3 c = vec3(tint_symbol);
  vec3 d = vec3((a * 2.0f));
}

