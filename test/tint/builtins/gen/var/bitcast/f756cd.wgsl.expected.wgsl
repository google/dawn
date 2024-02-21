fn bitcast_f756cd() {
  const arg_0 = vec3(1);
  var res : vec3<u32> = bitcast<vec3<u32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_f756cd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_f756cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_f756cd();
}
