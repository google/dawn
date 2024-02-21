enable f16;

fn bitcast_5081ed() {
  var res : vec3<f16> = bitcast<vec3<f16>>(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_5081ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_5081ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_5081ed();
}
