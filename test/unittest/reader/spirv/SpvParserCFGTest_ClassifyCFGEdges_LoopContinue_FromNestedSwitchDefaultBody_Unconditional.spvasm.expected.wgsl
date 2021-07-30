var<private> var_1 : u32;

fn main_1() {
  loop {
    if (false) {
    } else {
      break;
    }
    switch(42u) {
      default: {
        continue;
      }
      case 79u: {
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
