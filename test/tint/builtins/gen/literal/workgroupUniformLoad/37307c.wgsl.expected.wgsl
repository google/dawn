var<workgroup> arg_0 : u32;

fn workgroupUniformLoad_37307c() {
  var res : u32 = workgroupUniformLoad(&(arg_0));
}

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_37307c();
}
