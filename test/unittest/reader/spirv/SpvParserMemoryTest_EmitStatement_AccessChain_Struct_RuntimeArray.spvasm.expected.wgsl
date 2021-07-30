type RTArr = [[stride(4)]] array<f32>;

[[block]]
struct S {
  field0 : f32;
  age : RTArr;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  myvar.age[2u] = 42.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
