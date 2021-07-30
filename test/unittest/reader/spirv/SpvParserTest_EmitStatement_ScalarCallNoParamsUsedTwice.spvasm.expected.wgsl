fn x_50() -> u32 {
  return 42u;
}

fn x_100_1() {
  var x_10 : u32;
  let x_1 : u32 = x_50();
  x_10 = x_1;
  x_10 = x_1;
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
