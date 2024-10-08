SKIP: FAILED


cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
[numthreads(1, 1, 1)]
void main() {
  float3 v1 = (0.0f).xxx;
  v1[i[0u].x] = 1.0f;
}

FXC validation failure:
<scrubbed_path>(8,3-13): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
