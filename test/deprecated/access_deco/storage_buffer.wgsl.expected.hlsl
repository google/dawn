deprecated/access_deco/storage_buffer.wgsl:7:26 warning: use of deprecated language feature: declare access with var<storage, read_write> instead of using [[access]] decoration
[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;
                         ^^^

RWByteAddressBuffer sb : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  float x = asfloat(sb.Load(0u));
  return;
}
