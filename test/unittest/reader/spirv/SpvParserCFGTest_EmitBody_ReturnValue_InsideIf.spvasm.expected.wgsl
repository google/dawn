var<private> var_1 : u32;

fn x_200() -> u32 {
  if (false) {
    return 2u;
  }
  return 3u;
}

fn main_1() {
  let x_11 : u32 = x_200();
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
