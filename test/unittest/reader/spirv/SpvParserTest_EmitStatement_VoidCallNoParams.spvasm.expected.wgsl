fn x_50() {
  return;
}

fn x_100_1() {
  x_50();
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
