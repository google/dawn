SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static QuicksortObject obj = (QuicksortObject)0;
static BST tree[10] = (BST[10])0;
cbuffer cbuffer_x_50 : register(b0) {
  uint4 x_50[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST node, inout int data) {
  node.data = data;
  node.leftIndex = -1;
  node.rightIndex = -1;
}

void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = (BST)0;
  int param_1 = 0;
  BST param_2 = (BST)0;
  int param_3 = 0;
  baseIndex = 0;
  {
    while(true) {
      if ((baseIndex <= treeIndex)) {
      } else {
        break;
      }
      if ((data_1 <= tree[baseIndex].data)) {
        if ((tree[baseIndex].leftIndex == -1)) {
          int x_236 = baseIndex;
          tree[x_236].leftIndex = treeIndex;
          int x_239 = treeIndex;
          BST v = tree[x_239];
          param = v;
          param_1 = data_1;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          BST v_1 = param;
          tree[x_239] = v_1;
          return;
        } else {
          baseIndex = tree[baseIndex].leftIndex;
          {
          }
          continue;
        }
      } else {
        if ((tree[baseIndex].rightIndex == -1)) {
          int x_256 = baseIndex;
          tree[x_256].rightIndex = treeIndex;
          int x_259 = treeIndex;
          BST v_2 = tree[x_259];
          param_2 = v_2;
          param_3 = data_1;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST v_3 = param_2;
          tree[x_259] = v_3;
          return;
        } else {
          baseIndex = tree[baseIndex].rightIndex;
          {
          }
          continue;
        }
      }
      /* unreachable */
    }
  }
}

int identity_i1_(inout int a) {
  int x_202 = a;
  obj.numbers[x_202] = a;
  int x_206 = obj.numbers[2];
  return x_206;
}

int search_i1_(inout int t) {
  int index = 0;
  BST currentNode = (BST)0;
  int x_270 = 0;
  index = 0;
  {
    while(true) {
      if ((index != -1)) {
      } else {
        break;
      }
      BST v_4 = tree[index];
      currentNode = v_4;
      if ((currentNode.data == t)) {
        int x_287 = t;
        return x_287;
      }
      if ((t > currentNode.data)) {
        x_270 = currentNode.rightIndex;
      } else {
        x_270 = currentNode.leftIndex;
      }
      index = x_270;
      {
      }
      continue;
    }
  }
  return -1;
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (-2147483648))) : (2147483647));
}

void main_1() {
  int treeIndex_1 = 0;
  BST param_4 = (BST)0;
  int param_5 = 0;
  int param_6 = 0;
  int param_7 = 0;
  int param_8 = 0;
  int param_9 = 0;
  int param_10 = 0;
  int param_11 = 0;
  int param_12 = 0;
  int param_13 = 0;
  int param_14 = 0;
  int param_15 = 0;
  int param_16 = 0;
  int param_17 = 0;
  int param_18 = 0;
  int param_19 = 0;
  int param_20 = 0;
  int param_21 = 0;
  int param_22 = 0;
  int param_23 = 0;
  int pp = 0;
  int looplimiter0 = 0;
  int i = 0;
  int param_24 = 0;
  int count = 0;
  int i_1 = 0;
  int result = 0;
  int param_25 = 0;
  treeIndex_1 = 0;
  BST v_5 = tree[0];
  param_4 = v_5;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  BST v_6 = param_4;
  tree[0] = v_6;
  treeIndex_1 = (treeIndex_1 + 1);
  param_6 = treeIndex_1;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  treeIndex_1 = (treeIndex_1 + 1);
  param_8 = treeIndex_1;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  treeIndex_1 = (treeIndex_1 + 1);
  param_10 = treeIndex_1;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  treeIndex_1 = (treeIndex_1 + 1);
  param_12 = treeIndex_1;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  treeIndex_1 = (treeIndex_1 + 1);
  param_14 = treeIndex_1;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  treeIndex_1 = (treeIndex_1 + 1);
  param_16 = treeIndex_1;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  treeIndex_1 = (treeIndex_1 + 1);
  param_18 = treeIndex_1;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  treeIndex_1 = (treeIndex_1 + 1);
  param_20 = treeIndex_1;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  treeIndex_1 = (treeIndex_1 + 1);
  param_22 = treeIndex_1;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  pp = 0;
  looplimiter0 = 0;
  i = 0;
  {
    while(true) {
      if ((i < 10000)) {
      } else {
        break;
      }
      int v_7 = looplimiter0;
      if ((v_7 >= tint_f32_to_i32(asfloat(x_50[0u].y)))) {
        param_24 = (1 + tint_f32_to_i32(asfloat(x_50[0u].y)));
        int x_159 = identity_i1_(param_24);
        pp = x_159;
        break;
      }
      looplimiter0 = (looplimiter0 + 1);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((pp != 2)) {
    return;
  }
  count = 0;
  i_1 = 0;
  {
    while(true) {
      if ((i_1 < 20)) {
      } else {
        break;
      }
      param_25 = i_1;
      int x_176 = search_i1_(param_25);
      result = x_176;
      int x_177 = i_1;
      switch(x_177) {
        case 2:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 12:
        case 13:
        case 15:
        case 17:
        {
          if ((result == i_1)) {
            count = (count + 1);
          }
          break;
        }
        default:
        {
          if ((result == -1)) {
            count = (count + 1);
          }
          break;
        }
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  if ((count == 20)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 1.0f, 1.0f);
  }
}

main_out main_inner() {
  main_1();
  main_out v_8 = {x_GLF_color};
  return v_8;
}

main_outputs main() {
  main_out v_9 = main_inner();
  main_outputs v_10 = {v_9.x_GLF_color_1};
  return v_10;
}

FXC validation failure:
C:\src\dawn\Shader@0x000001F8B4A7FED0(40,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
C:\src\dawn\Shader@0x000001F8B4A7FED0(40,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
C:\src\dawn\Shader@0x000001F8B4A7FED0(89,3-20): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\dawn\Shader@0x000001F8B4A7FED0(206,5-15): error X3511: forced to unroll loop, but unrolling failed.

