SKIP: FAILED

#version 310 es

struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


BST tree[10] = BST[10](BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0));
vec4 x_GLF_color = vec4(0.0f);
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST node, inout int data) {
  int x_158 = data;
  node.data = x_158;
  node.leftIndex = -1;
  node.rightIndex = -1;
}
void insert_i1_i1_(inout int treeIndex, inout int data_1) {
  int baseIndex = 0;
  BST param = BST(0, 0, 0);
  int param_1 = 0;
  BST param_2 = BST(0, 0, 0);
  int param_3 = 0;
  baseIndex = 0;
  {
    while(true) {
      int x_167 = baseIndex;
      int x_168 = treeIndex;
      if ((x_167 <= x_168)) {
      } else {
        break;
      }
      int x_171 = data_1;
      int x_172 = baseIndex;
      int x_174 = tree[x_172].data;
      if ((x_171 <= x_174)) {
        int x_179 = baseIndex;
        int x_181 = tree[x_179].leftIndex;
        if ((x_181 == -1)) {
          int x_186 = baseIndex;
          int x_187 = treeIndex;
          tree[x_186].leftIndex = x_187;
          int x_189 = treeIndex;
          BST x_191 = tree[x_189];
          param = x_191;
          int x_192 = data_1;
          param_1 = x_192;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          BST x_194 = param;
          tree[x_189] = x_194;
          return;
        } else {
          int x_196 = baseIndex;
          int x_198 = tree[x_196].leftIndex;
          baseIndex = x_198;
          {
          }
          continue;
        }
      } else {
        int x_199 = baseIndex;
        int x_201 = tree[x_199].rightIndex;
        if ((x_201 == -1)) {
          int x_206 = baseIndex;
          int x_207 = treeIndex;
          tree[x_206].rightIndex = x_207;
          int x_209 = treeIndex;
          BST x_211 = tree[x_209];
          param_2 = x_211;
          int x_212 = data_1;
          param_3 = x_212;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST x_214 = param_2;
          tree[x_209] = x_214;
          return;
        } else {
          int x_216 = baseIndex;
          int x_218 = tree[x_216].rightIndex;
          baseIndex = x_218;
          {
          }
          continue;
        }
      }
      /* unreachable */
    }
  }
}
int search_i1_(inout int t) {
  int index = 0;
  BST currentNode = BST(0, 0, 0);
  int x_220 = 0;
  index = 0;
  {
    while(true) {
      int x_225 = index;
      if ((x_225 != -1)) {
      } else {
        break;
      }
      int x_228 = index;
      BST x_230 = tree[x_228];
      currentNode = x_230;
      int x_232 = currentNode.data;
      int x_233 = t;
      if ((x_232 == x_233)) {
        int x_237 = t;
        return x_237;
      }
      int x_238 = t;
      int x_240 = currentNode.data;
      if ((x_238 > x_240)) {
        int x_246 = currentNode.rightIndex;
        x_220 = x_246;
      } else {
        int x_248 = currentNode.leftIndex;
        x_220 = x_248;
      }
      int x_249 = x_220;
      index = x_249;
      {
      }
      continue;
    }
  }
  return -1;
}
void main_1() {
  int treeIndex_1 = 0;
  BST param_4 = BST(0, 0, 0);
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
  int count = 0;
  int i = 0;
  int result = 0;
  int param_24 = 0;
  treeIndex_1 = 0;
  BST x_84 = tree[0];
  param_4 = x_84;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  BST x_86 = param_4;
  tree[0] = x_86;
  int x_88 = treeIndex_1;
  treeIndex_1 = (x_88 + 1);
  int x_90 = treeIndex_1;
  param_6 = x_90;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  int x_92 = treeIndex_1;
  treeIndex_1 = (x_92 + 1);
  int x_94 = treeIndex_1;
  param_8 = x_94;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  int x_96 = treeIndex_1;
  treeIndex_1 = (x_96 + 1);
  int x_98 = treeIndex_1;
  param_10 = x_98;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  int x_100 = treeIndex_1;
  treeIndex_1 = (x_100 + 1);
  int x_102 = treeIndex_1;
  param_12 = x_102;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  int x_104 = treeIndex_1;
  treeIndex_1 = (x_104 + 1);
  int x_106 = treeIndex_1;
  param_14 = x_106;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  int x_108 = treeIndex_1;
  treeIndex_1 = (x_108 + 1);
  int x_110 = treeIndex_1;
  param_16 = x_110;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  int x_112 = treeIndex_1;
  treeIndex_1 = (x_112 + 1);
  int x_114 = treeIndex_1;
  param_18 = x_114;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  int x_116 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  int x_118 = treeIndex_1;
  param_20 = x_118;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  int x_120 = treeIndex_1;
  treeIndex_1 = (x_120 + 1);
  int x_122 = treeIndex_1;
  param_22 = x_122;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  count = 0;
  i = 0;
  {
    while(true) {
      int x_128 = i;
      if ((x_128 < 20)) {
      } else {
        break;
      }
      int x_131 = i;
      param_24 = x_131;
      int x_132 = search_i1_(param_24);
      result = x_132;
      int x_133 = i;
      switch(x_133) {
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
          int x_143 = result;
          int x_144 = i;
          if ((x_143 == x_144)) {
            int x_148 = count;
            count = (x_148 + 1);
          }
          break;
        }
        default:
        {
          int x_137 = result;
          if ((x_137 == -1)) {
            int x_141 = count;
            count = (x_141 + 1);
          }
          break;
        }
      }
      {
        int x_150 = i;
        i = (x_150 + 1);
      }
      continue;
    }
  }
  int x_152 = count;
  if ((x_152 == 20)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
