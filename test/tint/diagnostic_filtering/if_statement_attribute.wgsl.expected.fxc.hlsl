diagnostic_filtering/if_statement_attribute.wgsl:8:14 warning: 'dpdx' must only be called from uniform control flow
  } else if (dpdx(1.0) > 0)  {
             ^^^^^^^^^

diagnostic_filtering/if_statement_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

diagnostic_filtering/if_statement_attribute.wgsl:7:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  if ((x > 0.0f)) {
  } else {
    if ((ddx(1.0f) > 0.0f)) {
    }
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
