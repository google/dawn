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


BST tree_1[10] = BST[10](BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0), BST(0, 0, 0));
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void makeTreeNode_struct_BST_i1_i1_i11_i1_(inout BST tree, inout int data) {
  int x_169 = data;
  tree.data = x_169;
  tree.leftIndex = -1;
  tree.rightIndex = -1;
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
      int x_178 = baseIndex;
      int x_179 = treeIndex;
      if ((x_178 <= x_179)) {
      } else {
        break;
      }
      int x_182 = data_1;
      int x_183 = baseIndex;
      int x_185 = tree_1[x_183].data;
      if ((x_182 <= x_185)) {
        int x_190 = baseIndex;
        int x_192 = tree_1[x_190].leftIndex;
        if ((x_192 == -1)) {
          int x_197 = baseIndex;
          int x_198 = treeIndex;
          tree_1[x_197].leftIndex = x_198;
          int x_200 = treeIndex;
          BST x_202 = tree_1[x_200];
          param = x_202;
          int x_203 = data_1;
          param_1 = x_203;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param, param_1);
          BST x_205 = param;
          tree_1[x_200] = x_205;
          return;
        } else {
          int x_207 = baseIndex;
          int x_209 = tree_1[x_207].leftIndex;
          baseIndex = x_209;
          {
          }
          continue;
        }
      } else {
        int x_210 = baseIndex;
        int x_212 = tree_1[x_210].rightIndex;
        if ((x_212 == -1)) {
          int x_217 = baseIndex;
          int x_218 = treeIndex;
          tree_1[x_217].rightIndex = x_218;
          int x_220 = treeIndex;
          BST x_222 = tree_1[x_220];
          param_2 = x_222;
          int x_223 = data_1;
          param_3 = x_223;
          makeTreeNode_struct_BST_i1_i1_i11_i1_(param_2, param_3);
          BST x_225 = param_2;
          tree_1[x_220] = x_225;
          return;
        } else {
          int x_227 = baseIndex;
          int x_229 = tree_1[x_227].rightIndex;
          baseIndex = x_229;
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
  int x_231 = 0;
  index = 0;
  {
    while(true) {
      int x_236 = index;
      if ((x_236 != -1)) {
      } else {
        break;
      }
      int x_239 = index;
      BST x_241 = tree_1[x_239];
      currentNode = x_241;
      int x_243 = currentNode.data;
      int x_244 = t;
      if ((x_243 == x_244)) {
        int x_248 = t;
        return x_248;
      }
      int x_249 = t;
      int x_251 = currentNode.data;
      if ((x_249 > x_251)) {
        int x_257 = currentNode.rightIndex;
        x_231 = x_257;
      } else {
        int x_259 = currentNode.leftIndex;
        x_231 = x_259;
      }
      int x_260 = x_231;
      index = x_260;
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
  BST x_88 = tree_1[0];
  param_4 = x_88;
  param_5 = 9;
  makeTreeNode_struct_BST_i1_i1_i11_i1_(param_4, param_5);
  BST x_90 = param_4;
  tree_1[0] = x_90;
  int x_92 = treeIndex_1;
  treeIndex_1 = (x_92 + 1);
  int x_94 = treeIndex_1;
  param_6 = x_94;
  param_7 = 5;
  insert_i1_i1_(param_6, param_7);
  int x_96 = treeIndex_1;
  treeIndex_1 = (x_96 + 1);
  int x_98 = treeIndex_1;
  param_8 = x_98;
  param_9 = 12;
  insert_i1_i1_(param_8, param_9);
  int x_100 = treeIndex_1;
  treeIndex_1 = (x_100 + 1);
  int x_102 = treeIndex_1;
  param_10 = x_102;
  param_11 = 15;
  insert_i1_i1_(param_10, param_11);
  int x_104 = treeIndex_1;
  treeIndex_1 = (x_104 + 1);
  int x_106 = treeIndex_1;
  param_12 = x_106;
  param_13 = 7;
  insert_i1_i1_(param_12, param_13);
  int x_108 = treeIndex_1;
  treeIndex_1 = (x_108 + 1);
  int x_110 = treeIndex_1;
  param_14 = x_110;
  param_15 = 8;
  insert_i1_i1_(param_14, param_15);
  int x_112 = treeIndex_1;
  treeIndex_1 = (x_112 + 1);
  int x_114 = treeIndex_1;
  param_16 = x_114;
  param_17 = 2;
  insert_i1_i1_(param_16, param_17);
  int x_116 = treeIndex_1;
  treeIndex_1 = (x_116 + 1);
  int x_118 = treeIndex_1;
  param_18 = x_118;
  param_19 = 6;
  insert_i1_i1_(param_18, param_19);
  int x_120 = treeIndex_1;
  treeIndex_1 = (x_120 + 1);
  int x_122 = treeIndex_1;
  param_20 = x_122;
  param_21 = 17;
  insert_i1_i1_(param_20, param_21);
  int x_124 = treeIndex_1;
  treeIndex_1 = (x_124 + 1);
  int x_126 = treeIndex_1;
  param_22 = x_126;
  param_23 = 13;
  insert_i1_i1_(param_22, param_23);
  count = 0;
  i = 0;
  {
    while(true) {
      int x_132 = i;
      if ((x_132 < 20)) {
      } else {
        break;
      }
      bool x_155 = false;
      bool x_156_phi = false;
      int x_135 = i;
      param_24 = x_135;
      int x_136 = search_i1_(param_24);
      result = x_136;
      int x_137 = i;
      switch(x_137) {
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
          int x_147 = result;
          int x_148 = i;
          bool x_149 = (x_147 == x_148);
          x_156_phi = x_149;
          if (!(x_149)) {
            float x_154 = tint_symbol.x;
            x_155 = (x_154 < 0.0f);
            x_156_phi = x_155;
          }
          bool x_156 = x_156_phi;
          if (x_156) {
            int x_159 = count;
            count = (x_159 + 1);
          }
          break;
        }
        default:
        {
          int x_141 = result;
          if ((x_141 == -1)) {
            int x_145 = count;
            count = (x_145 + 1);
          }
          break;
        }
      }
      {
        int x_161 = i;
        i = (x_161 + 1);
      }
      continue;
    }
  }
  int x_163 = count;
  if ((x_163 == 20)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
