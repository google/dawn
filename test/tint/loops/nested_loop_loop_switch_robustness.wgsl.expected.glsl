#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((i < 2)) {
      } else {
        break;
      }
      {
        uvec2 tint_loop_idx_1 = uvec2(4294967295u);
        int j = 0;
        while(true) {
          if (all(equal(tint_loop_idx_1, uvec2(0u)))) {
            break;
          }
          if ((j < 2)) {
          } else {
            break;
          }
          bool tint_continue = false;
          switch(i) {
            case 0:
            {
              tint_continue = true;
              break;
            }
            default:
            {
              break;
            }
          }
          if (tint_continue) {
            {
              uint tint_low_inc_1 = (tint_loop_idx_1.x - 1u);
              tint_loop_idx_1.x = tint_low_inc_1;
              uint tint_carry_1 = uint((tint_low_inc_1 == 4294967295u));
              tint_loop_idx_1.y = (tint_loop_idx_1.y - tint_carry_1);
              uint v = uint(j);
              j = int((v + uint(2)));
            }
            continue;
          }
          {
            uint tint_low_inc_1 = (tint_loop_idx_1.x - 1u);
            tint_loop_idx_1.x = tint_low_inc_1;
            uint tint_carry_1 = uint((tint_low_inc_1 == 4294967295u));
            tint_loop_idx_1.y = (tint_loop_idx_1.y - tint_carry_1);
            uint v = uint(j);
            j = int((v + uint(2)));
          }
          continue;
        }
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        uint v_1 = uint(i);
        i = int((v_1 + uint(2)));
      }
      continue;
    }
  }
}
