var<private> position : vec4<f32>;

var<private> pos : u32;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_22 : vec4<f32> = position;
  gl_Position = x_22;
  pos = 0u;
  return;
}

struct main_out {
  [[builtin(position)]]
  gl_Position : vec4<f32>;
  [[location(0)]]
  pos_1 : u32;
};

[[stage(vertex)]]
fn main([[location(0)]] position_param : vec4<f32>) -> main_out {
  position = position_param;
  main_1();
  return main_out(gl_Position, pos);
}
