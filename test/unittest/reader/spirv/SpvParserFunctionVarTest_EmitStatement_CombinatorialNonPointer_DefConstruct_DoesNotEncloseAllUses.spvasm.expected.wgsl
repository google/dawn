struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

var<private> x_1 : u32;

fn main_1() {
  x_1 = 0u;
  loop {
    var x_2 : u32;
    x_1 = 1u;
    if (false) {
      break;
    }
    x_1 = 3u;
    if (true) {
      x_2 = (1u + 1u);
    } else {
      return;
    }
    x_1 = x_2;

    continuing {
      x_1 = 4u;
      if (false) {
        break;
      }
    }
  }
  x_1 = 5u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
