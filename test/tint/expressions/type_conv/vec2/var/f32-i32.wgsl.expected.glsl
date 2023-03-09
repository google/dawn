#version 310 es

ivec2 tint_select(ivec2 param_0, ivec2 param_1, bvec2 param_2) {
    return ivec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec2 tint_ftoi(vec2 v) {
  return tint_select(ivec2(2147483647), tint_select(ivec2(v), ivec2((-2147483647 - 1)), lessThan(v, vec2(-2147483648.0f))), lessThan(v, vec2(2147483520.0f)));
}

vec2 u = vec2(1.0f);
void f() {
  ivec2 v = tint_ftoi(u);
}

