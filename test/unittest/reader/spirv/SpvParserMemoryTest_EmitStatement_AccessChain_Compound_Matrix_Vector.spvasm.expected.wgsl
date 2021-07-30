var<private> myvar : mat3x4<f32>;

fn main_1() {
  myvar[2u].w = 42.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
