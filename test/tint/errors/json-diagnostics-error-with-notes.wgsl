// flags: --diagnostics-format=json

var<private> cond: bool;

fn foo() {
  if cond {
    workgroupBarrier();
  }
}
