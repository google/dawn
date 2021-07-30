struct strct {
  field0 : f32;
};

fn main_1() {
  var myvar : strct;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
