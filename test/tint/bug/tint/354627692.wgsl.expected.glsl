#version 310 es

layout(binding = 0, std430)
buffer buffer_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = v.inner;
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        {
          uvec2 tint_loop_idx_1 = uvec2(4294967295u);
          while(true) {
            if (all(equal(tint_loop_idx_1, uvec2(0u)))) {
              break;
            }
            if ((i > 5)) {
              uint v_1 = uint(i);
              i = int((v_1 * uint(2)));
              break;
            } else {
              uint v_2 = uint(i);
              i = int((v_2 * uint(2)));
              break;
            }
            /* unreachable */
          }
        }
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  v.inner = i;
}
