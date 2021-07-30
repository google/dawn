var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 20u: {
      fallthrough;
    }
    default: {
      fallthrough;
    }
    case 30u: {
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
