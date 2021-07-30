var<private> var_1 : u32;

fn main_1() {
  loop {
    var_1 = 1u;
    if (false) {
      break;
    }
    var_1 = 3u;

    continuing {
      var_1 = 2u;
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
