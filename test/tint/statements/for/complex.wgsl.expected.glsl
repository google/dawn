#version 310 es

void some_loop_body() {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int j = 0;
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      bool v = false;
      if ((i < 5)) {
        v = (j < 10);
      } else {
        v = false;
      }
      if (v) {
      } else {
        break;
      }
      some_loop_body();
      uint v_1 = uint(i);
      j = int((v_1 * uint(30)));
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        uint v_2 = uint(i);
        i = int((v_2 + uint(1)));
      }
      continue;
    }
  }
}
