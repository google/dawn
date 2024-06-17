SKIP: FAILED

float4 main() {
  int v1 = 1;
  uint v2 = 1u;
  float v3 = 1.0f;
  int3 v4 = (1).xxx;
  uint3 v5 = (1u).xxx;
  float3 v6 = (1.0f).xxx;
  float3x3 v7 = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
  float[10] v9 = (float[10])0;
  return (0.0f).xxxx;
}

DXC validation failure:
hlsl.hlsl:9:15: error: brackets are not allowed here; to declare an array, place the brackets after the name
  float[10] v9 = (float[10])0;
       ~~~~   ^
              [10]

