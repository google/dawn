fn another_function() {
  return;
}

fn x_100_1() {
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
