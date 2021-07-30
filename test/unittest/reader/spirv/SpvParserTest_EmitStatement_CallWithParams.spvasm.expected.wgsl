fn x_50(x_51 : u32, x_52 : u32) -> u32 {
  return (x_51 + x_52);
}

fn x_100_1() {
  let x_1 : u32 = x_50(42u, 84u);
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
