struct S {
  field0 : f32;
  age : f32;
};

var<private> myvar : S;

fn main_1() {
  myvar.age = 42.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
