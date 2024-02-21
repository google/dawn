enable f16;

fn bitcast_23c8bd() {
  var arg_0 = vec2<f16>(1.0h);
  var res : f32 = bitcast<f32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_23c8bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_23c8bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_23c8bd();
}
