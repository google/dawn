struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_1 : u32;

var<private> x_7 : bool;

var<private> x_8 : bool;

fn main_1() {
  let x_101 : bool = x_7;
  let x_102 : bool = x_8;
  loop {
    var x_2_phi : u32;
    if (x_101) {
      break;
    }
    if (x_102) {
      x_2_phi = 0u;
      continue;
    } else {
      x_2_phi = 1u;
      continue;
    }
    x_2_phi = 0u;

    continuing {
      let x_2 : u32 = x_2_phi;
      x_1 = x_2;
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
