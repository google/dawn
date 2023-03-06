enable f16;

fn select_ed7c13() {
  var res : vec2<f16> = select(vec2<f16>(1.0h), vec2<f16>(1.0h), vec2<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_ed7c13();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_ed7c13();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_ed7c13();
}
