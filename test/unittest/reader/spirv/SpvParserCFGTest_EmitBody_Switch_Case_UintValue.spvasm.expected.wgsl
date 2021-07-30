var<private> var_1 : u32;

fn main_1() {
  var_1 = 1u;
  switch(42u) {
    case 50u: {
      var_1 = 40u;
    }
    case 2000000000u: {
      var_1 = 30u;
    }
    case 20u: {
      var_1 = 20u;
    }
    default: {
    }
  }
  var_1 = 7u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
