var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 20u: {
      if (false) {
        break;
      }
    }
    default: {
      fallthrough;
    }
    case 50u: {
      if (false) {
      } else {
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
