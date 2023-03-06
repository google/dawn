enable f16;

fn distance_7d201f() {
  var res : f16 = distance(1.0h, 1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_7d201f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_7d201f();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_7d201f();
}
