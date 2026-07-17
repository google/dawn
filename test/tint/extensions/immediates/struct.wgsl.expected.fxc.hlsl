
cbuffer cbuffer_data : register(b0, space1) {
  uint4 data[1];
};
RWByteAddressBuffer output : register(u0);
[numthreads(1, 1, 1)]
void main() {
  float3 v = asfloat(data[0u].xyz);
  output.Store4(0u, asuint(float4(v, float(data[0u].w))));
}

