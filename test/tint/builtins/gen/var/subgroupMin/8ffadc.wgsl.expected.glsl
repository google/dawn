SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupMin_8ffadc() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_8ffadc();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_8ffadc();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/8ffadc.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupMin_8ffadc() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_8ffadc();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_8ffadc();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/8ffadc.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
