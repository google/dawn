#version 310 es

uvec3 tint_select(uvec3 param_0, uvec3 param_1, bvec3 param_2) {
    return uvec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec3 tint_ftou(vec3 v) {
  return tint_select(uvec3(4294967295u), tint_select(uvec3(v), uvec3(0u), lessThan(v, vec3(0.0f))), lessThan(v, vec3(4294967040.0f)));
}

vec3 u = vec3(1.0f);
void f() {
  uvec3 v = tint_ftou(u);
}

