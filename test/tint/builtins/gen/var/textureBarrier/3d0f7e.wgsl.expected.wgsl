enable chromium_experimental_read_write_storage_texture;

fn textureBarrier_3d0f7e() {
  textureBarrier();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureBarrier_3d0f7e();
}
