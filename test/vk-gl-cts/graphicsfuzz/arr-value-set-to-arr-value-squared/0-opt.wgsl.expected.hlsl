SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

static QuicksortObject obj = (QuicksortObject)0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_30 : register(b0, space0) {
  uint4 x_30[1];
};

void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  const int x_92 = i;
  const int x_94 = obj.numbers[x_92];
  temp = x_94;
  const int x_95 = i;
  const int x_96 = j;
  const int x_98 = obj.numbers[x_96];
  obj.numbers[x_95] = x_98;
  const int x_100 = j;
  obj.numbers[x_100] = temp;
  return;
}

int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  const int x_104 = h;
  const int x_106 = obj.numbers[x_104];
  pivot = x_106;
  const int x_107 = l;
  i_1 = (x_107 - 1);
  const int x_109 = l;
  j_1 = x_109;
  [loop] while (true) {
    const int x_114 = j_1;
    const int x_115 = h;
    if ((x_114 <= (x_115 - 1))) {
    } else {
      break;
    }
    const int x_121 = obj.numbers[j_1];
    if ((x_121 <= pivot)) {
      i_1 = (i_1 + 1);
      param = i_1;
      param_1 = j_1;
      swap_i1_i1_(param, param_1);
    }
    {
      j_1 = (j_1 + 1);
    }
  }
  param_2 = (i_1 + 1);
  const int x_135 = h;
  param_3 = x_135;
  swap_i1_i1_(param_2, param_3);
  return (i_1 + 1);
}

void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = (int[10])0;
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  const int x_141 = (top + 1);
  top = x_141;
  stack[x_141] = l_1;
  const int x_145 = (top + 1);
  top = x_145;
  stack[x_145] = h_1;
  [loop] while (true) {
    if ((top >= 0)) {
    } else {
      break;
    }
    const int x_155 = top;
    top = (x_155 - 1);
    const int x_158 = stack[x_155];
    h_1 = x_158;
    const int x_159 = top;
    top = (x_159 - 1);
    const int x_162 = stack[x_159];
    l_1 = x_162;
    param_4 = l_1;
    param_5 = h_1;
    const int x_165 = performPartition_i1_i1_(param_4, param_5);
    p = x_165;
    if (((p - 1) > l_1)) {
      const int x_173 = (top + 1);
      top = x_173;
      stack[x_173] = l_1;
      const int x_177 = (top + 1);
      top = x_177;
      stack[x_177] = (p - 1);
    }
    if (((p + 1) < h_1)) {
      const int x_188 = (top + 1);
      top = x_188;
      stack[x_188] = (p + 1);
      const int x_193 = (top + 1);
      top = x_193;
      stack[x_193] = h_1;
    }
  }
  return;
}

void main_1() {
  int i_2 = 0;
  i_2 = 0;
  {
    [loop] for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      obj.numbers[i_2] = (10 - i_2);
      const int x_71 = i_2;
      const int x_74 = obj.numbers[i_2];
      const int x_77 = obj.numbers[i_2];
      obj.numbers[x_71] = (x_74 * x_77);
    }
  }
  quicksort_();
  const int x_84 = obj.numbers[0];
  const int x_86 = obj.numbers[4];
  if ((x_84 < x_86)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 1.0f, 0.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x000001BEDA24CD50(124,7-22): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x000001BEDA24CD50(123,12-45): error X3531: can't unroll loops marked with loop attribute

