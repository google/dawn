enable f16;

fn abs_fd247f() {
  var res : f16 = abs(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_fd247f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_fd247f();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_fd247f();
}
