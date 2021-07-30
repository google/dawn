type RTArr = [[stride(4)]] array<u32>;

[[block]]
struct S {
  field0 : u32;
  field1 : RTArr;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  myvar.field1[1u] = 0u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
