
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3 v = m[int(1)];
  float f = v.y;
  s.Store(0u, asuint(f));
}

