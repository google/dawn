struct InnerS {
  int v;
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s1 : register(u0);

void s1_store(uint offset, InnerS value) {
  s1.Store((offset + 0u), asuint(value.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  s1_store((4u * uniforms[0].x), v);
  return;
}
