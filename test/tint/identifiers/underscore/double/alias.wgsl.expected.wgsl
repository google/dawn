type a = i32;

type a__ = i32;

type b = a;

type b__ = a__;

fn f() {
  var c : b;
  var d : b__;
}
