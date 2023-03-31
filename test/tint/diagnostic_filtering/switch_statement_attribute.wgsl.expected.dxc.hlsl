diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:24 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                       ^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

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
