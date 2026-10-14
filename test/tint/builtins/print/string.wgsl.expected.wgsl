@compute @workgroup_size(1)
fn compute_main() {
  print(`hello world`);
}

@fragment
fn fragment_main() {
  print(`hello from fragment`);
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print(`hello from vertex`);
  return out;
}
