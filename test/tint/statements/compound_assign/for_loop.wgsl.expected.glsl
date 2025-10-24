#version 310 es

uint i = 0u;
int idx1() {
  i = (i + 1u);
  return 1;
}
int idx2() {
  i = (i + 2u);
  return 1;
}
int idx3() {
  i = (i + 3u);
  return 1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    uint v_1 = min(uint(idx1()), 3u);
    a[v_1] = (a[v_1] * 2.0f);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      uint v_2 = min(uint(idx2()), 3u);
      if ((a[v_2] < 10.0f)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        uint v_3 = min(uint(idx3()), 3u);
        a[v_3] = (a[v_3] + 1.0f);
      }
      continue;
    }
  }
}
