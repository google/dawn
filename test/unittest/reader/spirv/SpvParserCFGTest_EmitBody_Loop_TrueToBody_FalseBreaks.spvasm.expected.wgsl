var<private> var_1 : u32;

fn main_1() {
  loop {
    var_1 = 1u;
    if (false) {
    } else {
      break;
    }
    var_1 = 2u;

    continuing {
      var_1 = 3u;
    }
  }
  var_1 = 4u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
