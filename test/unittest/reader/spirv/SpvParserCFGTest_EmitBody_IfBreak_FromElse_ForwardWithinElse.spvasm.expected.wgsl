var<private> var_1 : u32;

fn main_1() {
  var_1 = 1u;
  var guard10 : bool = true;
  if (false) {
    var_1 = 2u;
    guard10 = false;
  } else {
    if (guard10) {
      var_1 = 3u;
      if (true) {
        guard10 = false;
      }
      if (guard10) {
        var_1 = 4u;
        guard10 = false;
      }
    }
  }
  var_1 = 5u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
