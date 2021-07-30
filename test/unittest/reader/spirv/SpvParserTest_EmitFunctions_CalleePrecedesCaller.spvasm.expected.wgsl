fn leaf() -> u32 {
  return 0u;
}

fn branch() -> u32 {
  let leaf_result : u32 = leaf();
  return leaf_result;
}

fn root() {
  let branch_result : u32 = branch();
  return;
}

fn x_100_1() {
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
