var<private> var_1 : u32;

fn main_1() {
  var_1 = 1u;
  switch(42u) {
    case 20u: {
      var_1 = 20u;
      if (false) {
        var_1 = 40u;
        break;
      }
      var_1 = 50u;
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
