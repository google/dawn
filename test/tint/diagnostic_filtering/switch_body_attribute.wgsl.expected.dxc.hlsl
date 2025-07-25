<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:5:11 warning: 'dpdx' must only be called from uniform control flow
      _ = dpdx(1.0);
          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
              ^

struct main_inputs {
  float x : TEXCOORD0;
};


int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

void main_inner(float x) {
  switch(tint_f32_to_i32(x)) {
    default:
    {
      ddx(1.0f);
      break;
    }
  }
}

void main(main_inputs inputs) {
  main_inner(inputs.x);
}

