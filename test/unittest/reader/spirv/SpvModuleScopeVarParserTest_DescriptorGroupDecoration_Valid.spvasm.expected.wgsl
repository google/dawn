type Arr = [[stride(4)]] array<u32, 2>;

[[block]]
struct S {
  field0 : u32;
  field1 : f32;
  field2 : Arr;
};

[[group(3), binding(9)]] var<storage, read_write> x_1 : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
