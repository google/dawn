[[group(0), binding(0)]] var t_rgba8unorm_r : texture_storage_2d_array<rgba8unorm, read>;

[[group(0), binding(1)]] var t_rgba8snorm_r : texture_storage_2d_array<rgba8snorm, read>;

[[group(0), binding(2)]] var t_rgba8uint_r : texture_storage_2d_array<rgba8uint, read>;

[[group(0), binding(3)]] var t_rgba8sint_r : texture_storage_2d_array<rgba8sint, read>;

[[group(0), binding(4)]] var t_rgba16uint_r : texture_storage_2d_array<rgba16uint, read>;

[[group(0), binding(5)]] var t_rgba16sint_r : texture_storage_2d_array<rgba16sint, read>;

[[group(0), binding(6)]] var t_rgba16float_r : texture_storage_2d_array<rgba16float, read>;

[[group(0), binding(7)]] var t_r32uint_r : texture_storage_2d_array<r32uint, read>;

[[group(0), binding(8)]] var t_r32sint_r : texture_storage_2d_array<r32sint, read>;

[[group(0), binding(9)]] var t_r32float_r : texture_storage_2d_array<r32float, read>;

[[group(0), binding(10)]] var t_rg32uint_r : texture_storage_2d_array<rg32uint, read>;

[[group(0), binding(11)]] var t_rg32sint_r : texture_storage_2d_array<rg32sint, read>;

[[group(0), binding(12)]] var t_rg32float_r : texture_storage_2d_array<rg32float, read>;

[[group(0), binding(13)]] var t_rgba32uint_r : texture_storage_2d_array<rgba32uint, read>;

[[group(0), binding(14)]] var t_rgba32sint_r : texture_storage_2d_array<rgba32sint, read>;

[[group(0), binding(15)]] var t_rgba32float_r : texture_storage_2d_array<rgba32float, read>;

[[group(0), binding(50)]] var t_rgba8unorm_w : texture_storage_2d_array<rgba8unorm, write>;

[[group(0), binding(51)]] var t_rgba8snorm_w : texture_storage_2d_array<rgba8snorm, write>;

[[group(0), binding(52)]] var t_rgba8uint_w : texture_storage_2d_array<rgba8uint, write>;

[[group(0), binding(53)]] var t_rgba8sint_w : texture_storage_2d_array<rgba8sint, write>;

[[group(0), binding(54)]] var t_rgba16uint_w : texture_storage_2d_array<rgba16uint, write>;

[[group(0), binding(55)]] var t_rgba16sint_w : texture_storage_2d_array<rgba16sint, write>;

[[group(0), binding(56)]] var t_rgba16float_w : texture_storage_2d_array<rgba16float, write>;

[[group(0), binding(57)]] var t_r32uint_w : texture_storage_2d_array<r32uint, write>;

[[group(0), binding(58)]] var t_r32sint_w : texture_storage_2d_array<r32sint, write>;

[[group(0), binding(59)]] var t_r32float_w : texture_storage_2d_array<r32float, write>;

[[group(0), binding(60)]] var t_rg32uint_w : texture_storage_2d_array<rg32uint, write>;

[[group(0), binding(61)]] var t_rg32sint_w : texture_storage_2d_array<rg32sint, write>;

[[group(0), binding(62)]] var t_rg32float_w : texture_storage_2d_array<rg32float, write>;

[[group(0), binding(63)]] var t_rgba32uint_w : texture_storage_2d_array<rgba32uint, write>;

[[group(0), binding(64)]] var t_rgba32sint_w : texture_storage_2d_array<rgba32sint, write>;

[[group(0), binding(65)]] var t_rgba32float_w : texture_storage_2d_array<rgba32float, write>;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(t_rgba8unorm_r);
  ignore(t_rgba8snorm_r);
  ignore(t_rgba8uint_r);
  ignore(t_rgba8sint_r);
  ignore(t_rgba16uint_r);
  ignore(t_rgba16sint_r);
  ignore(t_rgba16float_r);
  ignore(t_r32uint_r);
  ignore(t_r32sint_r);
  ignore(t_r32float_r);
  ignore(t_rg32uint_r);
  ignore(t_rg32sint_r);
  ignore(t_rg32float_r);
  ignore(t_rgba32uint_r);
  ignore(t_rgba32sint_r);
  ignore(t_rgba32float_r);
  ignore(t_rgba8unorm_w);
  ignore(t_rgba8snorm_w);
  ignore(t_rgba8uint_w);
  ignore(t_rgba8sint_w);
  ignore(t_rgba16uint_w);
  ignore(t_rgba16sint_w);
  ignore(t_rgba16float_w);
  ignore(t_r32uint_w);
  ignore(t_r32sint_w);
  ignore(t_r32float_w);
  ignore(t_rg32uint_w);
  ignore(t_rg32sint_w);
  ignore(t_rg32float_w);
  ignore(t_rgba32uint_w);
  ignore(t_rgba32sint_w);
  ignore(t_rgba32float_w);
}
