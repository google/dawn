fn print_1a5163() {
  var arg_0 = vec2<i32>(1i);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_1a5163();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_1a5163();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_1a5163();
  return out;
}
