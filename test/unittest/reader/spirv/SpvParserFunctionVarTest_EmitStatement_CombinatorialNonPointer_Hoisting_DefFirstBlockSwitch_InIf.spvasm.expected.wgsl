struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_200 : u32;

fn main_1() {
  if (true) {
    let x_1 : u32 = 1u;
    switch(1u) {
      case 0u: {
      }
      default: {
      }
    }
    let x_3 : u32 = x_1;
    x_200 = x_3;
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
