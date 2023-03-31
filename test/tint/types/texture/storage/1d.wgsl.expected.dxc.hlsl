RWTexture1D<float4> t_rgba8unorm : register(u0);
RWTexture1D<float4> t_rgba8snorm : register(u1);
RWTexture1D<uint4> t_rgba8uint : register(u2);
RWTexture1D<int4> t_rgba8sint : register(u3);
RWTexture1D<uint4> t_rgba16uint : register(u4);
RWTexture1D<int4> t_rgba16sint : register(u5);
RWTexture1D<float4> t_rgba16float : register(u6);
RWTexture1D<uint4> t_r32uint : register(u7);
RWTexture1D<int4> t_r32sint : register(u8);
RWTexture1D<float4> t_r32float : register(u9);
RWTexture1D<uint4> t_rg32uint : register(u10);
RWTexture1D<int4> t_rg32sint : register(u11);
RWTexture1D<float4> t_rg32float : register(u12);
RWTexture1D<uint4> t_rgba32uint : register(u13);
RWTexture1D<int4> t_rgba32sint : register(u14);
RWTexture1D<float4> t_rgba32float : register(u15);

[numthreads(1, 1, 1)]
void main() {
  return;
}
