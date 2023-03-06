enable f16;

fn saturate_cd2028() {
  var arg_0 = vec2<f16>(2.0h);
  var res : vec2<f16> = saturate(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_cd2028();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_cd2028();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_cd2028();
}
