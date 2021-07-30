var<private> var_1 : u32;

fn main_1() {
  switch(42u) {
    default: {
    }
    case 20u: {
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
