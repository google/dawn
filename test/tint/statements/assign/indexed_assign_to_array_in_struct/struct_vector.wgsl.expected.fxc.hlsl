struct OuterS {
  float3 v1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  uint v = uniforms[0u].x;
  float3 v_1 = s1.v1;
  float3 v_2 = float3((1.0f).xxx);
  uint3 v_3 = uint3((v).xxx);
  s1.v1 = (((v_3 == uint3(0u, 1u, 2u))) ? (v_2) : (v_1));
}

