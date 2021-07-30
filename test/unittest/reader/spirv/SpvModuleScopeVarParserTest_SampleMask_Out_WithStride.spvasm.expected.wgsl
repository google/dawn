type Arr = [[stride(4)]] array<u32, 1>;

type Arr_1 = [[stride(4)]] array<u32, 2>;

type Arr_2 = [[stride(4)]] array<i32, 1>;

type Arr_3 = [[stride(4)]] array<i32, 2>;

var<private> x_1 : Arr;

fn main_1() {
  x_1[0] = 0u;
  return;
}

struct main_out {
  [[builtin(sample_mask)]]
  x_1_1 : u32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1[0]);
}
