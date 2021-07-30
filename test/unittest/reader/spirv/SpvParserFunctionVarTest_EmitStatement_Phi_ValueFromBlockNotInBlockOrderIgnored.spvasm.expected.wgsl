struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  loop {
    if (false) {
      break;
    }
    break;

    continuing {
      var x_81_phi : f32;
      let x_81 : f32 = x_81_phi;
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
