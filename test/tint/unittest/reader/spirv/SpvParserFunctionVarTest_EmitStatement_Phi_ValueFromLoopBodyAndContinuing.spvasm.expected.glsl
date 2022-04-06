SKIP: FAILED


struct S {
  field0 : u32,
  field1 : f32,
  field2 : array<u32, 2u>,
}

var<private> x_1 : u32;

var<private> x_17 : bool;

fn main_1() {
  let x_101 : bool = x_17;
  loop {
    var x_2_phi : u32;
    var x_5_phi : u32;
    x_2_phi = 0u;
    x_5_phi = 1u;
    loop {
      var x_7 : u32;
      let x_2 : u32 = x_2_phi;
      let x_5 : u32 = x_5_phi;
      let x_4 : u32 = (x_2 + 1u);
      let x_6 : u32 = (x_4 + 1u);
      if (x_101) {
        break;
      }

      continuing {
        x_7 = (x_4 + x_6);
        x_2_phi = x_4;
        x_5_phi = x_7;
      }
    }
  }
  return;
}

@stage(fragment)
fn main() {
  main_1();
}

error: loop does not exit
