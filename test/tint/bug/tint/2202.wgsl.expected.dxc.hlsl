<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^


[numthreads(1, 1, 1)]
void main() {
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      {
        uint2 tint_loop_idx_1 = (4294967295u).xx;
        while(true) {
          if (all((tint_loop_idx_1 == (0u).xx))) {
            break;
          }
          return;
        }
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
      }
    }
  }
  /* unreachable */
}

