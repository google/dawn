struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_1 : u32;

var<private> x_7 : bool;

var<private> x_8 : bool;

fn main_1() {
  loop {
    var x_2_phi : u32;
    var x_3_phi : u32;
    let x_101 : bool = x_7;
    let x_102 : bool = x_8;
    x_2_phi = 0u;
    x_3_phi = 1u;
    if (x_101) {
      break;
    }
    loop {
      let x_2 : u32 = x_2_phi;
      let x_3 : u32 = x_3_phi;
      x_2_phi = (x_2 + 1u);
      x_3_phi = x_3;
      if (x_102) {
        break;
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
