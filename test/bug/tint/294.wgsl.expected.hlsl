bug/tint/294.wgsl:8:24 warning: use of deprecated language feature: declare access with var<storage, read> instead of using [[access]] decoration
[[set(0), binding(1)]] var<storage> lights : [[access(read)]] Lights;
                       ^^^

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

ByteAddressBuffer lights : register(t1, space0);
