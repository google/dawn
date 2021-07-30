type RTArr = [[stride(4)]] array<u32>;

[[block]]
struct S {
  field0 : u32;
  field1 : RTArr;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
