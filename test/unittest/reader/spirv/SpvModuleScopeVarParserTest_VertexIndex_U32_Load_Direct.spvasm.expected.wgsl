var<private> x_4 : u32;

var<private> x_1 : vec4<f32>;

fn main_1() {
  let x_2 : u32 = x_4;
  return;
}

struct main_out {
  [[builtin(position)]]
  x_1_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] x_4_param : u32) -> main_out {
  x_4 = x_4_param;
  main_1();
  return main_out(x_1);
}
