struct str {
  int arr[4];
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared str S;
void func() {
  int v[4] = (int[4])0;
  S.arr = v;
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 4u)) {
        break;
      }
      S.arr[v_1] = int(0);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  func();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

