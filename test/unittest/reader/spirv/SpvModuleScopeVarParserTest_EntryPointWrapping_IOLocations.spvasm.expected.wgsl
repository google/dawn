var<private> x_1 : u32;

var<private> x_2 : u32;

var<private> x_3 : u32;

var<private> x_4 : u32;

fn main_1() {
  return;
}

struct main_out {
  [[location(0)]]
  x_2_1 : u32;
  [[location(6)]]
  x_4_1 : u32;
};

[[stage(fragment)]]
fn main([[location(0)]] x_1_param : u32, [[location(30)]] x_3_param : u32) -> main_out {
  x_1 = x_1_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_2, x_4);
}
