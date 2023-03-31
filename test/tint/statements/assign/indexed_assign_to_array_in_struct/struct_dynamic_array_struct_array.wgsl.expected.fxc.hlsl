struct InnerS {
  int v;
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s : register(u0);

void s_store(uint offset, InnerS value) {
  s.Store((offset + 0u), asuint(value.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  s_store(((32u * uniforms[0].x) + (4u * uniforms[0].y)), v);
  return;
}
