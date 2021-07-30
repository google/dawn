var<private> var_1 : u32;

fn main_1() {
  var_1 = 0u;
  loop {
    var_1 = 1u;
    if (true) {
      var_1 = 2u;
      if (false) {
        continue;
      } else {
        break;
      }
    }
    var_1 = 3u;

    continuing {
      var_1 = 4u;
    }
  }
  var_1 = 5u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
