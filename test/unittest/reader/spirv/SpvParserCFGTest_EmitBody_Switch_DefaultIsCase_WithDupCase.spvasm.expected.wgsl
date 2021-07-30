var<private> var_1 : u32;

fn main_1() {
  var_1 = 1u;
  switch(42u) {
    case 40u: {
      var_1 = 40u;
    }
    case 20u: {
      var_1 = 20u;
    }
    default: {
      fallthrough;
    }
    case 30u: {
      var_1 = 30u;
    }
  }
  var_1 = 7u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
