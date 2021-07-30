var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 20u: {
    }
    default: {
      fallthrough;
    }
    case 30u: {
      fallthrough;
    }
    case 40u: {
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
