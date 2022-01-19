SKIP: FAILED


var<private> var_1 : u32;

fn main_1() {
  var_1 = 10u;
  loop {
    var_1 = 20u;
    if (false) {
      var_1 = 30u;
      continue;
    } else {
      var_1 = 40u;
    }

    continuing {
      var_1 = 90u;
    }
  }
  var_1 = 99u;
  return;
}

@stage(fragment)
fn main() {
  main_1();
}

error: loop does not exit
