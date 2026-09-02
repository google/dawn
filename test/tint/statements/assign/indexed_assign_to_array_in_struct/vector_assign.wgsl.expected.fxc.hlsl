struct OuterS {
  uint a1[8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
uint f(uint i) {
  return (i + 1u);
}

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  float3 v = (0.0f).xxx;
  uint v_1 = min(uniforms[0u].x, 7u);
  float3 v_2 = v;
  v = (((uint3((s1.a1[v_1]).xxx) == uint3(0u, 1u, 2u))) ? ((1.0f).xxx) : (v_2));
  uint v_3 = min(uniforms[0u].x, 7u);
  uint v_4 = f(s1.a1[v_3]);
  float3 v_5 = v;
  v = (((uint3((v_4).xxx) == uint3(0u, 1u, 2u))) ? ((1.0f).xxx) : (v_5));
}

