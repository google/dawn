type Arr = array<u32, 50>;

[[block]]
struct Buf {
  count : u32;
  data : Arr;
};

[[group(0), binding(0)]] var<storage> b : [[access(read)]] Buf;

[[stage(compute)]]
fn main() {
  var i : u32 = 0u;
  loop {
    if ((i >= b.count)) {
      break;
    }
    let p : ptr<storage, u32> = &(b.data[i]);
    if (((i % 2u) == 0u)) {
      continue;
    }
    *(p) = 0u;

    continuing {
      *(p) = (*(p) * 2u);
      i = (i + 1u);
    }
  }
  return;
}
