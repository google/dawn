[[block]]
struct S {
  a : f32;
};

[[group(0), binding(0)]] var<storage> b0 : S;

[[group(1), binding(0)]] var<storage> b1 : S;

[[group(2), binding(0)]] var<storage> b2 : S;

[[group(3), binding(0)]] var<storage> b3 : S;

[[group(4), binding(0)]] var<storage> b4 : S;

[[group(5), binding(0)]] var<storage> b5 : S;

[[group(6), binding(0)]] var<storage> b6 : S;

[[group(7), binding(0)]] var<storage> b7 : S;

[[group(9), binding(1)]] var<uniform> b8 : S;

[[group(8), binding(1)]] var<uniform> b9 : S;

[[group(10), binding(1)]] var<uniform> b10 : S;

[[group(11), binding(1)]] var<uniform> b11 : S;

[[group(12), binding(1)]] var<uniform> b12 : S;

[[group(13), binding(1)]] var<uniform> b13 : S;

[[group(14), binding(1)]] var<uniform> b14 : S;

[[group(15), binding(1)]] var<uniform> b15 : S;

[[group(0), binding(1)]] var t0 : texture_2d<f32>;

[[group(1), binding(1)]] var t1 : texture_2d<f32>;

[[group(2), binding(1)]] var t2 : texture_2d<f32>;

[[group(3), binding(1)]] var t3 : texture_2d<f32>;

[[group(4), binding(1)]] var t4 : texture_2d<f32>;

[[group(5), binding(1)]] var t5 : texture_2d<f32>;

[[group(6), binding(1)]] var t6 : texture_2d<f32>;

[[group(7), binding(1)]] var t7 : texture_2d<f32>;

[[group(8), binding(200)]] var t8 : texture_depth_2d;

[[group(9), binding(200)]] var t9 : texture_depth_2d;

[[group(10), binding(200)]] var t10 : texture_depth_2d;

[[group(11), binding(200)]] var t11 : texture_depth_2d;

[[group(12), binding(200)]] var t12 : texture_depth_2d;

[[group(13), binding(200)]] var t13 : texture_depth_2d;

[[group(14), binding(200)]] var t14 : texture_depth_2d;

[[group(15), binding(200)]] var t15 : texture_depth_2d;

[[group(0), binding(200)]] var s0 : sampler;

[[group(1), binding(200)]] var s1 : sampler;

[[group(2), binding(200)]] var s2 : sampler;

[[group(3), binding(200)]] var s3 : sampler;

[[group(4), binding(200)]] var s4 : sampler;

[[group(5), binding(200)]] var s5 : sampler;

[[group(6), binding(200)]] var s6 : sampler;

[[group(7), binding(200)]] var s7 : sampler;

[[group(8), binding(300)]] var s8 : sampler_comparison;

[[group(9), binding(300)]] var s9 : sampler_comparison;

[[group(10), binding(300)]] var s10 : sampler_comparison;

[[group(11), binding(300)]] var s11 : sampler_comparison;

[[group(12), binding(300)]] var s12 : sampler_comparison;

[[group(13), binding(300)]] var s13 : sampler_comparison;

[[group(14), binding(300)]] var s14 : sampler_comparison;

[[group(15), binding(300)]] var s15 : sampler_comparison;

[[stage(fragment)]]
fn main() {
  ignore(b0);
  ignore(b1);
  ignore(b2);
  ignore(b3);
  ignore(b4);
  ignore(b5);
  ignore(b6);
  ignore(b7);
  ignore(b8);
  ignore(b9);
  ignore(b10);
  ignore(b11);
  ignore(b12);
  ignore(b13);
  ignore(b14);
  ignore(b15);
  ignore(t0);
  ignore(t1);
  ignore(t2);
  ignore(t3);
  ignore(t4);
  ignore(t5);
  ignore(t6);
  ignore(t7);
  ignore(t8);
  ignore(t9);
  ignore(t10);
  ignore(t11);
  ignore(t12);
  ignore(t13);
  ignore(t14);
  ignore(t15);
  ignore(s0);
  ignore(s1);
  ignore(s2);
  ignore(s3);
  ignore(s4);
  ignore(s5);
  ignore(s6);
  ignore(s7);
  ignore(s8);
  ignore(s9);
  ignore(s10);
  ignore(s11);
  ignore(s12);
  ignore(s13);
  ignore(s14);
  ignore(s15);
}
