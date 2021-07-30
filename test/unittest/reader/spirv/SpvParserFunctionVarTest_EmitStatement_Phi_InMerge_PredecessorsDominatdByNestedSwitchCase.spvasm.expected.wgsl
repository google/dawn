struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_1 : u32;

var<private> x_7 : bool;

var<private> x_8 : bool;

fn main_1() {
  var x_41_phi : u32;
  switch(1u) {
    default: {
      fallthrough;
    }
    case 0u: {
      fallthrough;
    }
    case 1u: {
      if (true) {
      } else {
        x_41_phi = 0u;
        break;
      }
      x_41_phi = 1u;
    }
  }
  let x_41 : u32 = x_41_phi;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
