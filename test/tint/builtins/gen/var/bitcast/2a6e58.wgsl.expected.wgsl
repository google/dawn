enable f16;

fn bitcast_2a6e58() {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec2<f32> = bitcast<vec2<f32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_2a6e58();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_2a6e58();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_2a6e58();
}
