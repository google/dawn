fn print_7afd23() {
  var arg_0 = vec3<u32>(1u);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_7afd23();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_7afd23();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_7afd23();
  return out;
}
