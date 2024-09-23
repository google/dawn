SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapX_a4e103() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_a4e103();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_a4e103();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/a4e103.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapX_a4e103() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_a4e103();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_a4e103();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/a4e103.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
