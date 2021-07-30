fn x_200() -> f32 {
  return 0.0;
}

fn x_100_1() {
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
