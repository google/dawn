struct Output {
  [[builtin(position)]]
  Position : vec4<f32>;
  [[location(0)]]
  color : vec4<f32>;
};

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] VertexIndex : u32, [[builtin(instance_index)]] InstanceIndex : u32) -> Output {
  let zv : array<vec2<f32>, 4> = array<vec2<f32>, 4>(vec2<f32>(0.200000003, 0.200000003), vec2<f32>(0.300000012, 0.300000012), vec2<f32>(-0.100000001, -0.100000001), vec2<f32>(1.100000024, 1.100000024));
  let z : f32 = zv[InstanceIndex].x;
  var output : Output;
  output.Position = vec4<f32>(0.5, 0.5, z, 1.0);
  let colors : array<vec4<f32>, 4> = array<vec4<f32>, 4>(vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  output.color = colors[InstanceIndex];
  return output;
}
