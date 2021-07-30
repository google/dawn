type RTArr = [[stride(4)]] array<u32>;

[[block]]
struct S {
  first : u32;
  rtarr : RTArr;
};

[[group(0), binding(0)]] var<storage, read_write> myvar : S;

fn main_1() {
  let x_1 : u32 = arrayLength(&(myvar.rtarr));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
