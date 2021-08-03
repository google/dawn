[[block]]
struct S {
  field0 : [[stride(64)]] array<vec2<f32>, 3>;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
