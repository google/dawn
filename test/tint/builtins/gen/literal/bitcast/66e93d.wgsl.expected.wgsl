enable f16;

fn bitcast_66e93d() {
  var res : vec2<f16> = bitcast<vec2<f16>>(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_66e93d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_66e93d();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_66e93d();
}
