type Arr = [[stride(4)]] array<u32, 1>;

type Arr_1 = [[stride(4)]] array<u32, 2>;

type Arr_2 = [[stride(4)]] array<i32, 1>;

type Arr_3 = [[stride(4)]] array<i32, 2>;

var<private> x_1 : Arr;

fn main_1() {
  let x_3 : u32 = x_1[0];
  return;
}

[[stage(fragment)]]
fn main([[builtin(sample_mask)]] x_1_param : u32) {
  x_1[0] = x_1_param;
  main_1();
}
