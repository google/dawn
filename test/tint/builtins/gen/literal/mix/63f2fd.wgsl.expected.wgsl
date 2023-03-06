enable f16;

fn mix_63f2fd() {
  var res : vec3<f16> = mix(vec3<f16>(1.0h), vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_63f2fd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_63f2fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_63f2fd();
}
