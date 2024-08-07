fn transpose_84a763() {
  var res = transpose(mat2x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@fragment
fn fragment_main() {
  transpose_84a763();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_84a763();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  transpose_84a763();
  return out;
}
