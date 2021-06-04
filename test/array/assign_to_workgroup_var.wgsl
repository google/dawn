type ArrayType = [[stride(16)]] array<i32, 4>;

[[block]]
struct S {
  arr : ArrayType;
};

var<private> src_private : ArrayType;
var<workgroup> src_workgroup : ArrayType;
[[group(0), binding(0)]] var<uniform> src_uniform : S;
[[group(0), binding(1)]] var<storage, read_write> src_storage : S;

var<workgroup> dst : ArrayType;
var<workgroup> dst_nested : array<array<array<i32, 2>, 3>, 4>;

fn ret_arr() -> ArrayType {
  return ArrayType();
}

fn ret_struct_arr() -> S {
  return S();
}

fn foo(src_param : ArrayType) {
  var src_function : ArrayType;

  // Assign from type constructor.
  dst = ArrayType(1, 2, 3, 3);

  // Assign from parameter.
  dst = src_param;

  // Assign from function call.
  dst = ret_arr();

  // Assign from constant.
  let src_let : ArrayType = ArrayType();
  dst = src_let;

  // Assign from var, various storage classes.
  dst = src_function;
  dst = src_private;
  dst = src_workgroup;

  // Assign from struct.arr, various storage classes.
  dst = ret_struct_arr().arr;
  dst = src_uniform.arr;
  dst = src_storage.arr;

  // Nested assignment.
  var src_nested : array<array<array<i32, 2>, 3>, 4>;
  dst_nested = src_nested;
}
