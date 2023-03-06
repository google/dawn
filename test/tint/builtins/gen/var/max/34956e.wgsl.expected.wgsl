enable f16;

fn max_34956e() {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = vec2<f16>(1.0h);
  var res : vec2<f16> = max(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_34956e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_34956e();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_34956e();
}
