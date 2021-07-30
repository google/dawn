var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 30u: {
    }
    case 20u: {
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
