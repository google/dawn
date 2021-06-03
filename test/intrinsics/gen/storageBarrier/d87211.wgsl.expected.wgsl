fn storageBarrier_d87211() {
  storageBarrier();
}

[[stage(compute)]]
fn compute_main() {
  storageBarrier_d87211();
}
