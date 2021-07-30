warning: use of deprecated intrinsic
[[group(0), binding(0)]] var x_20 : texture_storage_1d<rg32float, read>;

fn main_1() {
  let x_125 : vec4<f32> = textureLoad(x_20, i32(1u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
