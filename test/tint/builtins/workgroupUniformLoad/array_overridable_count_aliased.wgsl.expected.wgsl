const wgsize : i32 = 64i;

alias Array = array<i32, (wgsize * 2)>;

var<workgroup> v : Array;

fn foo() -> i32 {
  return workgroupUniformLoad(&(v))[0];
}
