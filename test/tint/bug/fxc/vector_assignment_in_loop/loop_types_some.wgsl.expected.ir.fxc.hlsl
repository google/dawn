
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = (0.0f).xx;
  float3 v3f = (0.0f).xxx;
  float4 v4f = (0.0f).xxxx;
  int2 v2i = (int(0)).xx;
  int3 v3i = (int(0)).xxx;
  int4 v4i = (int(0)).xxxx;
  uint2 v2u = (0u).xx;
  uint3 v3u = (0u).xxx;
  uint4 v4u = (0u).xxxx;
  bool2 v2b = (false).xx;
  bool3 v3b = (false).xxx;
  bool4 v4b = (false).xxxx;
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      v2f[i] = 1.0f;
      v2i[i] = int(1);
      v2u[i] = 1u;
      v2b[i] = true;
      {
        i = (i + int(1));
      }
      continue;
    }
  }
  int i = int(0);
  v3f[i] = 1.0f;
  v4f[i] = 1.0f;
  v3i[i] = int(1);
  v4i[i] = int(1);
  v3u[i] = 1u;
  v4u[i] = 1u;
  v3b[i] = true;
  v4b[i] = true;
}

