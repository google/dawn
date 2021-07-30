fn x_50() -> u32 {
  return 42u;
}

fn x_100_1() {
  let x_1 : u32 = x_50();
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
