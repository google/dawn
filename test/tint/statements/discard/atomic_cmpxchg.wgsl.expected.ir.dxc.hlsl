struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct foo_outputs {
  int tint_symbol : SV_Target0;
};


RWByteAddressBuffer a : register(u0);
static bool continue_execution = true;
int foo_inner() {
  continue_execution = false;
  int x = int(0);
  int v = int(0);
  a.InterlockedCompareExchange(int(0u), int(0), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == int(0))};
  atomic_compare_exchange_result_i32 result = v_2;
  if (result.exchanged) {
    atomic_compare_exchange_result_i32 v_3 = v_2;
    x = v_3.old_value;
  }
  return x;
}

foo_outputs foo() {
  foo_outputs v_4 = {foo_inner()};
  if (!(continue_execution)) {
    discard;
  }
  foo_outputs v_5 = v_4;
  return v_5;
}

