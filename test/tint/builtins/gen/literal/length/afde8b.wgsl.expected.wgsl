fn length_afde8b() {
  var res : f32 = length(vec2<f32>(0.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_afde8b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_afde8b();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_afde8b();
}
