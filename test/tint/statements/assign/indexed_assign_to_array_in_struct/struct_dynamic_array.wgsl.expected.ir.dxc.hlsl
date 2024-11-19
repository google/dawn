struct InnerS {
  int v;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s1 : register(u0);
void v_1(uint offset, InnerS obj) {
  s1.Store((offset + 0u), asuint(obj.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  InnerS v_2 = v;
  v_1((0u + (uniforms[0u].x * 4u)), v_2);
}

