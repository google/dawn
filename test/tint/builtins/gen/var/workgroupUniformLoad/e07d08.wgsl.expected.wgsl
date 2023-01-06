enable f16;

var<workgroup> arg_0 : f16;

fn workgroupUniformLoad_e07d08() {
  var res : f16 = workgroupUniformLoad(&(arg_0));
}

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_e07d08();
}
