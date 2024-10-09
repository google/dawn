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
  uint v_1 = uniforms[0u].x;
  float3 v_2 = v;
  float3 v_3 = s1.a1[v_1].xxx;
  v = (((v_3 == float3(int(0), int(1), int(2)))) ? (1.0f.xxx) : (v_2));
  uint v_4 = uniforms[0u].x;
  uint v_5 = f(s1.a1[v_4]);
  float3 v_6 = v;
  v = (((v_5.xxx == float3(int(0), int(1), int(2)))) ? (1.0f.xxx) : (v_6));
}

