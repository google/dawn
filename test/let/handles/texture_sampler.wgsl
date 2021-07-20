[[group(0), binding(0)]] var t : texture_2d<f32>;
[[group(0), binding(1)]] var s : sampler;

[[stage(fragment)]]
fn main() {
  let x = t;
  let a = s;
  ignore(1);
  let y = x;
  let b = a;
  ignore(2);
  let z = y;
  let c = b;
  ignore(textureSample(z, c, vec2<f32>(1., 2.)));
}
