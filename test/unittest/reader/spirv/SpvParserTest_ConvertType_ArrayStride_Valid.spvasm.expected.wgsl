type Arr = [[stride(8)]] array<u32, 5>;

fn x_100_1() {
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
