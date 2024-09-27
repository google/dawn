#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec3 tint_ftou(vec3 v) {
  return mix(uvec3(4294967295u), mix(uvec3(v), uvec3(0u), lessThan(v, vec3(0.0f))), lessThanEqual(v, vec3(4294967040.0f)));
}

vec3 u = vec3(1.0f);
void f() {
  uvec3 v = tint_ftou(u);
}

