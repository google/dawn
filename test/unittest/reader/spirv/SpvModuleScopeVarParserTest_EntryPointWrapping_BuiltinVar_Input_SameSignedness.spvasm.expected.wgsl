var<private> x_1 : u32;

var<private> x_4 : vec4<f32>;

fn main_1() {
  let x_2 : u32 = x_1;
  return;
}

struct main_out {
  [[builtin(position)]]
  x_4_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[builtin(instance_index)]] x_1_param : u32) -> main_out {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
