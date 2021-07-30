var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    case 20u: {
      fallthrough;
    }
    case 40u: {
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
