enable f16;

fn exp2_151a4c() {
  var res : vec2<f16> = exp2(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_151a4c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_151a4c();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_151a4c();
}
