enable f16;

fn bitcast_1df11f() {
  var res : vec2<f16> = bitcast<vec2<f16>>(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_1df11f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_1df11f();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_1df11f();
}
