fn print_2389f5() {
  print(vec3<i32>(1i));
}

@fragment
fn fragment_main() {
  print_2389f5();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_2389f5();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_2389f5();
  return out;
}
