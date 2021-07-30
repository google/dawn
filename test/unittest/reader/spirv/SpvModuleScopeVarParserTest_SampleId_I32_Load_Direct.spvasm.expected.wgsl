var<private> x_1 : i32;

fn main_1() {
  let x_2 : i32 = x_1;
  return;
}

[[stage(fragment)]]
fn main([[builtin(sample_index)]] x_1_param : u32) {
  x_1 = bitcast<i32>(x_1_param);
  main_1();
}
