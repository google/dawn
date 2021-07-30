var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 50u: {
      if (false) {
      }
    }
    case 20u: {
      if (false) {
      }
    }
    default: {
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
