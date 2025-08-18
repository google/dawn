fn v_1(v : ptr<function, vec2<f32>>) -> f32 {
  return (*(v)).x;
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var v_2 : vec2<f32>;
  v_1(&(v_2));
}
