struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_200 : S = S(1u, 1.5, array<u32, 2>(1u, 2u));

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
