[[group(0), binding(0)]] var x_20 : texture_storage_1d<rg32float, write>;

fn main_1() {
  textureStore(x_20, i32(1u), vec4<f32>(0.0, 0.0, 0.0, 0.0));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
