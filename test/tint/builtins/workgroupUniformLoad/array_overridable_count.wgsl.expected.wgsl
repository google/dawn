const wgsize : i32 = 64i;

var<workgroup> v : array<i32, (wgsize * 2)>;

fn foo() -> i32 {
  return workgroupUniformLoad(&(v))[0];
}
