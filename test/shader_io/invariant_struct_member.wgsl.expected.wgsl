struct Out {
  [[builtin(position), invariant]]
  pos : vec4<f32>;
};

[[stage(vertex)]]
fn main() -> Out {
  return Out();
}
