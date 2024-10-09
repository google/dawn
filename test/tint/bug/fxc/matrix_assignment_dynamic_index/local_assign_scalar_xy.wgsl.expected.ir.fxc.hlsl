
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
  uint v = uniforms[0u].x;
  uint v_1 = uniforms[0u].y;
  switch(v) {
    case 0u:
    {
      float4 v_2 = m1[0u];
      m1[0u] = (((v_1.xxxx == float4(int(0), int(1), int(2), int(3)))) ? (1.0f.xxxx) : (v_2));
      break;
    }
    case 1u:
    {
      float4 v_3 = m1[1u];
      m1[1u] = (((v_1.xxxx == float4(int(0), int(1), int(2), int(3)))) ? (1.0f.xxxx) : (v_3));
      break;
    }
    default:
    {
      break;
    }
  }
}

