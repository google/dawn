enable f16;

fn bitcast_436211() {
  var arg_0 = 1.0h;
  var res : f16 = bitcast<f16>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_436211();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_436211();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_436211();
}
