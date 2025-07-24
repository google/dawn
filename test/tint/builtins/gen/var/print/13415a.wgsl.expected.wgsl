fn print_13415a() {
  var arg_0 = vec4<i32>(1i);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_13415a();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_13415a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_13415a();
  return out;
}
