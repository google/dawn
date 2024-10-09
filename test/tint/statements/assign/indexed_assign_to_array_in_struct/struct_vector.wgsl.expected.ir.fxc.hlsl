struct OuterS {
  float3 v1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  float3 v = s1.v1;
  float3 v_1 = uniforms[0u].x.xxx;
  s1.v1 = (((v_1 == float3(int(0), int(1), int(2)))) ? (1.0f.xxx) : (v));
}

