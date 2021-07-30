var<private> myvar : vec4<u32>;

fn main_1() {
  myvar.z = 42u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
