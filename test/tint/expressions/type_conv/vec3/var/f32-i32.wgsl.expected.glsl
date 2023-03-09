#version 310 es

ivec3 tint_select(ivec3 param_0, ivec3 param_1, bvec3 param_2) {
    return ivec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec3 tint_ftoi(vec3 v) {
  return tint_select(ivec3(2147483647), tint_select(ivec3(v), ivec3((-2147483647 - 1)), lessThan(v, vec3(-2147483648.0f))), lessThan(v, vec3(2147483520.0f)));
}

vec3 u = vec3(1.0f);
void f() {
  ivec3 v = tint_ftoi(u);
}

