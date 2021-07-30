var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    default: {
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
