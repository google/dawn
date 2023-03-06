enable f16;

fn ldexp_7485ce() {
  var res : vec3<f16> = ldexp(vec3<f16>(1.0h), vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_7485ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_7485ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_7485ce();
}
