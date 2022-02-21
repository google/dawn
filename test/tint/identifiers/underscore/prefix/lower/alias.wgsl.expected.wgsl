type a = i32;

type _a = i32;

type b = a;

type _b = _a;

fn f() {
  var c : b;
  var d : _b;
}
