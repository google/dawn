diagnostic_filtering/default_case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/default_case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

diagnostic_filtering/default_case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

Texture2D<float4> t : register(t1, space0);
SamplerState s : register(s2, space0);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  do {
  } while (false);
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
