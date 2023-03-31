RWTexture3D<float4> t_rgba8unorm : register(u0);
RWTexture3D<float4> t_rgba8snorm : register(u1);
RWTexture3D<uint4> t_rgba8uint : register(u2);
RWTexture3D<int4> t_rgba8sint : register(u3);
RWTexture3D<uint4> t_rgba16uint : register(u4);
RWTexture3D<int4> t_rgba16sint : register(u5);
RWTexture3D<float4> t_rgba16float : register(u6);
RWTexture3D<uint4> t_r32uint : register(u7);
RWTexture3D<int4> t_r32sint : register(u8);
RWTexture3D<float4> t_r32float : register(u9);
RWTexture3D<uint4> t_rg32uint : register(u10);
RWTexture3D<int4> t_rg32sint : register(u11);
RWTexture3D<float4> t_rg32float : register(u12);
RWTexture3D<uint4> t_rgba32uint : register(u13);
RWTexture3D<int4> t_rgba32sint : register(u14);
RWTexture3D<float4> t_rgba32float : register(u15);

[numthreads(1, 1, 1)]
void main() {
  return;
}
