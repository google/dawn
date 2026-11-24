#version 310 es

void main_inner(uvec3 n) {
  int i = 0;
  bvec3 v = bvec3(true);
  bvec3 v_1 = bvec3(all(v));
  uint v_2 = uint(i);
  bool b = !(mix(mix(v_1.x, v_1.y, (v_2 == 1u)), v_1.z, (v_2 == 2u)));
  switch(i) {
    default:
    {
      break;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_NumWorkGroups);
}
