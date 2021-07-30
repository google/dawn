type Arr = [[stride(16)]] array<u32, 2>;

struct S {
  field0 : u32;
  field1 : f32;
  field2 : Arr;
};

fn main_1() {
  var x_200 : Arr = Arr(0u, 0u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
