#version 310 es

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec2 tint_ftou(vec2 v) {
  return tint_select(uvec2(4294967295u), tint_select(uvec2(v), uvec2(0u), lessThan(v, vec2(0.0f))), lessThan(v, vec2(4294967040.0f)));
}

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}

void f() {
  vec2 tint_symbol = m();
  uvec2 v = tint_ftou(tint_symbol);
}

