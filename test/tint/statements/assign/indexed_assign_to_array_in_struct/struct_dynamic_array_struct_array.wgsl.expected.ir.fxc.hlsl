struct InnerS {
  int v;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s : register(u0);
void v_1(uint offset, InnerS obj) {
  s.Store((offset + 0u), asuint(obj.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  uint v_2 = uniforms[0u].y;
  uint v_3 = (uint(uniforms[0u].x) * 32u);
  uint v_4 = (uint(v_2) * 4u);
  InnerS v_5 = v;
  v_1(((0u + v_3) + v_4), v_5);
}

