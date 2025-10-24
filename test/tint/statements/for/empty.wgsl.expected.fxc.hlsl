
[numthreads(1, 1, 1)]
void f() {
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      if (false) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
      }
      continue;
    }
  }
}

