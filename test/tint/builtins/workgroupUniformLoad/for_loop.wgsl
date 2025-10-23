var<workgroup> a : i32;
var<workgroup> b : i32;

@compute @workgroup_size(1)
fn foo() {
  for (var i = 0; i < workgroupUniformLoad(&a); i += workgroupUniformLoad(&b)) {
  }
}
