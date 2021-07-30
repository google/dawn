var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 20u: {
      if (false) {
      }
      if (false) {
        break;
      }
      fallthrough;
    }
    default: {
      fallthrough;
    }
    case 50u: {
      if (false) {
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
