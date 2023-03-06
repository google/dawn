enable f16;

fn radians_fbacf0() {
  var res : vec2<f16> = radians(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_fbacf0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_fbacf0();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_fbacf0();
}
