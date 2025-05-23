<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:4:9 warning: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:6:7 note: control flow depends on possibly non-uniform value
      break if x > 0.0;
      ^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:6:16 note: user-defined input 'x' of 'main' may be non-uniform
      break if x > 0.0;
               ^

struct main_inputs {
  float x : TEXCOORD0;
};


void main_inner(float x) {
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      ddx(1.0f);
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        if ((x > 0.0f)) { break; }
      }
      continue;
    }
  }
}

void main(main_inputs inputs) {
  main_inner(inputs.x);
}

