enable f16;

fn bitcast_674557() {
  var res : vec2<f16> = bitcast<vec2<f16>>(1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_674557();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_674557();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_674557();
}
