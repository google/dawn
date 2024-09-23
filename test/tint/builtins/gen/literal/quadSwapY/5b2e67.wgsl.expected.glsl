SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn quadSwapY_5b2e67() -> vec4<f16> {
  var res : vec4<f16> = quadSwapY(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_5b2e67();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_5b2e67();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/5b2e67.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn quadSwapY_5b2e67() -> vec4<f16> {
  var res : vec4<f16> = quadSwapY(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_5b2e67();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_5b2e67();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/5b2e67.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
