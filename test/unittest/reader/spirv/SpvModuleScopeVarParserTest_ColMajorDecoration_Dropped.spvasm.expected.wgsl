[[block]]
struct S {
  field0 : mat3x2<f32>;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
