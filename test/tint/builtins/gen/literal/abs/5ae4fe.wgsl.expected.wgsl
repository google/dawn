enable f16;

fn abs_5ae4fe() {
  var res : vec2<f16> = abs(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_5ae4fe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_5ae4fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_5ae4fe();
}
