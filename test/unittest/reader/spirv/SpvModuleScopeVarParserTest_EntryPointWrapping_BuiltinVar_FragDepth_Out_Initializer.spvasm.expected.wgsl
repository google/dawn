var<private> x_1 : f32 = 0.0;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(frag_depth)]]
  x_1_1 : f32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1);
}
