var<private> var_1 : u32;

fn main_1() {
  switch(20u) {
    case 20u: {
      if (false) {
      } else {
        break;
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
