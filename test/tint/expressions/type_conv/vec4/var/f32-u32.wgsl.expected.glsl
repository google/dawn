#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec4 tint_ftou(vec4 v) {
  return mix(uvec4(4294967295u), mix(uvec4(v), uvec4(0u), lessThan(v, vec4(0.0f))), lessThanEqual(v, vec4(4294967040.0f)));
}

vec4 u = vec4(1.0f);
void f() {
  uvec4 v = tint_ftou(u);
}

