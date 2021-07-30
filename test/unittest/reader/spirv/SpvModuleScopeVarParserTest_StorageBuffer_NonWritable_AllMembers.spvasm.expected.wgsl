[[block]]
struct S {
  field0 : f32;
  field1 : f32;
};

[[group(0), binding(0)]] var<storage, read> x_1 : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
