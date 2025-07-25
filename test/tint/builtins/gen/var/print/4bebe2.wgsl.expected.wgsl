fn print_4bebe2() {
  var arg_0 = vec3<f32>(1.0f);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_4bebe2();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_4bebe2();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_4bebe2();
  return out;
}
