RWTexture2D<float4> t_rgba8unorm : register(u0);
RWTexture2D<float4> t_rgba8snorm : register(u1);
RWTexture2D<uint4> t_rgba8uint : register(u2);
RWTexture2D<int4> t_rgba8sint : register(u3);
RWTexture2D<uint4> t_rgba16uint : register(u4);
RWTexture2D<int4> t_rgba16sint : register(u5);
RWTexture2D<float4> t_rgba16float : register(u6);
RWTexture2D<uint4> t_r32uint : register(u7);
RWTexture2D<int4> t_r32sint : register(u8);
RWTexture2D<float4> t_r32float : register(u9);
RWTexture2D<uint4> t_rg32uint : register(u10);
RWTexture2D<int4> t_rg32sint : register(u11);
RWTexture2D<float4> t_rg32float : register(u12);
RWTexture2D<uint4> t_rgba32uint : register(u13);
RWTexture2D<int4> t_rgba32sint : register(u14);
RWTexture2D<float4> t_rgba32float : register(u15);

[numthreads(1, 1, 1)]
void main() {
  return;
}
