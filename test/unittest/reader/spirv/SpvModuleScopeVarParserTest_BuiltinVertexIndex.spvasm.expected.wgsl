var<private> x_2 : u32;

var<private> position : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  position_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] x_2_param : u32) -> main_out {
  x_2 = x_2_param;
  main_1();
  return main_out(position);
}
