#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v;
int f() {
  return 0;
}
void g() {
  int j = 0;
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((j >= 1)) {
        break;
      }
      j = int((uint(j) + 1u));
      int k = f();
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
      }
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_1 = v.inner[0u];
  switch(int(v_1.x)) {
    case 0:
    {
      uvec4 v_2 = v.inner[0u];
      switch(int(v_2.x)) {
        case 0:
        {
          break;
        }
        default:
        {
          g();
          break;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}
