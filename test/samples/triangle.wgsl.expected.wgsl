let pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(vec2<f32>(0.0, 0.5), vec2<f32>(-0.5, -0.5), vec2<f32>(0.5, -0.5));

[[stage(vertex)]]
fn vtx_main([[builtin(vertex_index)]] VertexIndex : i32) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
}

[[stage(fragment)]]
fn frag_main() -> [[location(0)]] vec4<f32> {
  return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
