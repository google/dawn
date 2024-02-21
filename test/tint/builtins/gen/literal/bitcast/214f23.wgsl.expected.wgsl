enable f16;

fn bitcast_214f23() {
  var res : vec2<i32> = bitcast<vec2<i32>>(vec4<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_214f23();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_214f23();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_214f23();
}
