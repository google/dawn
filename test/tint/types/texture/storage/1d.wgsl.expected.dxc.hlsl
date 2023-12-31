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
  uint tint_tmp;
  t_rgba8unorm.GetDimensions(tint_tmp);
  uint dim1 = tint_tmp;
  uint tint_tmp_1;
  t_rgba8snorm.GetDimensions(tint_tmp_1);
  uint dim2 = tint_tmp_1;
  uint tint_tmp_2;
  t_rgba8uint.GetDimensions(tint_tmp_2);
  uint dim3 = tint_tmp_2;
  uint tint_tmp_3;
  t_rgba8sint.GetDimensions(tint_tmp_3);
  uint dim4 = tint_tmp_3;
  uint tint_tmp_4;
  t_rgba16uint.GetDimensions(tint_tmp_4);
  uint dim5 = tint_tmp_4;
  uint tint_tmp_5;
  t_rgba16sint.GetDimensions(tint_tmp_5);
  uint dim6 = tint_tmp_5;
  uint tint_tmp_6;
  t_rgba16float.GetDimensions(tint_tmp_6);
  uint dim7 = tint_tmp_6;
  uint tint_tmp_7;
  t_r32uint.GetDimensions(tint_tmp_7);
  uint dim8 = tint_tmp_7;
  uint tint_tmp_8;
  t_r32sint.GetDimensions(tint_tmp_8);
  uint dim9 = tint_tmp_8;
  uint tint_tmp_9;
  t_r32float.GetDimensions(tint_tmp_9);
  uint dim10 = tint_tmp_9;
  uint tint_tmp_10;
  t_rg32uint.GetDimensions(tint_tmp_10);
  uint dim11 = tint_tmp_10;
  uint tint_tmp_11;
  t_rg32sint.GetDimensions(tint_tmp_11);
  uint dim12 = tint_tmp_11;
  uint tint_tmp_12;
  t_rg32float.GetDimensions(tint_tmp_12);
  uint dim13 = tint_tmp_12;
  uint tint_tmp_13;
  t_rgba32uint.GetDimensions(tint_tmp_13);
  uint dim14 = tint_tmp_13;
  uint tint_tmp_14;
  t_rgba32sint.GetDimensions(tint_tmp_14);
  uint dim15 = tint_tmp_14;
  uint tint_tmp_15;
  t_rgba32float.GetDimensions(tint_tmp_15);
  uint dim16 = tint_tmp_15;
  return;
}
