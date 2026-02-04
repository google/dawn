enable chromium_experimental_resource_table;

@group(0) @binding(0) var<storage, read_write> o : u32;

@fragment
fn fs() {
  if (hasResource<sampler>(0)) {
    o += 1;
  }
  if (hasResource<sampler_comparison>(1)) {
    o += 1;
  }
}
