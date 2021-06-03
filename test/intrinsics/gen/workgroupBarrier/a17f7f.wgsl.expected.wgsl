fn workgroupBarrier_a17f7f() {
  workgroupBarrier();
}

[[stage(compute)]]
fn compute_main() {
  workgroupBarrier_a17f7f();
}
