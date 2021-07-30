var<private> myvar : mat3x4<f32>;

fn main_1() {
  myvar[2u] = vec4<f32>(42.0, 42.0, 42.0, 42.0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
