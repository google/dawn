enable f16;

var<workgroup> arg_0 : f16;

fn workgroupUniformLoad_e07d08() {
  var res : f16 = workgroupUniformLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_e07d08();
}
