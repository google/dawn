fn atanh_440cca() {
  var arg_0 = vec3<f32>(0.5f);
  var res : vec3<f32> = atanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_440cca();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_440cca();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_440cca();
}
