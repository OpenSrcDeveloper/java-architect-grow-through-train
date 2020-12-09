/*
 * Copyright (c) 2007, 2017, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include "precompiled.hpp"
#include "memory/allocation.inline.hpp"
#include "opto/connode.hpp"
#include "opto/mulnode.hpp"
#include "opto/subnode.hpp"
#include "opto/vectornode.hpp"
#include "utilities/powerOfTwo.hpp"
#include "utilities/globalDefinitions.hpp"

//------------------------------VectorNode--------------------------------------

// Return the vector operator for the specified scalar operation
// and vector length.
int VectorNode::opcode(int sopc, BasicType bt) {
  switch (sopc) {
  case Op_AddI:
    switch (bt) {
    case T_BOOLEAN:
    case T_BYTE:      return Op_AddVB;
    case T_CHAR:
    case T_SHORT:     return Op_AddVS;
    case T_INT:       return Op_AddVI;
    default:          ShouldNotReachHere(); return 0;
    }
  case Op_AddL:
    assert(bt == T_LONG, "must be");
    return Op_AddVL;
  case Op_AddF:
    assert(bt == T_FLOAT, "must be");
    return Op_AddVF;
  case Op_AddD:
    assert(bt == T_DOUBLE, "must be");
    return Op_AddVD;
  case Op_SubI:
    switch (bt) {
    case T_BOOLEAN:
    case T_BYTE:   return Op_SubVB;
    case T_CHAR:
    case T_SHORT:  return Op_SubVS;
    case T_INT:    return Op_SubVI;
    default:       ShouldNotReachHere(); return 0;
    }
  case Op_SubL:
    assert(bt == T_LONG, "must be");
    return Op_SubVL;
  case Op_SubF:
    assert(bt == T_FLOAT, "must be");
    return Op_SubVF;
  case Op_SubD:
    assert(bt == T_DOUBLE, "must be");
    return Op_SubVD;
  case Op_MulI:
    switch (bt) {
    case T_BOOLEAN:return 0;
    case T_BYTE:   return Op_MulVB;
    case T_CHAR:
    case T_SHORT:  return Op_MulVS;
    case T_INT:    return Op_MulVI;
    default:       ShouldNotReachHere(); return 0;
    }
  case Op_MulL:
    assert(bt == T_LONG, "must be");
    return Op_MulVL;
  case Op_MulF:
    assert(bt == T_FLOAT, "must be");
    return Op_MulVF;
  case Op_MulD:
    assert(bt == T_DOUBLE, "must be");
    return Op_MulVD;
  case Op_FmaD:
    assert(bt == T_DOUBLE, "must be");
    return Op_FmaVD;
  case Op_FmaF:
    assert(bt == T_FLOAT, "must be");
    return Op_FmaVF;
  case Op_CMoveF:
    assert(bt == T_FLOAT, "must be");
    return Op_CMoveVF;
  case Op_CMoveD:
    assert(bt == T_DOUBLE, "must be");
    return Op_CMoveVD;
  case Op_DivF:
    assert(bt == T_FLOAT, "must be");
    return Op_DivVF;
  case Op_DivD:
    assert(bt == T_DOUBLE, "must be");
    return Op_DivVD;
  case Op_AbsI:
    switch (bt) {
    case T_BOOLEAN:
    case T_CHAR:  return 0; // abs does not make sense for unsigned
    case T_BYTE:  return Op_AbsVB;
    case T_SHORT: return Op_AbsVS;
    case T_INT:   return Op_AbsVI;
    default: ShouldNotReachHere(); return 0;
    }
  case Op_AbsL:
    assert(bt == T_LONG, "must be");
    return Op_AbsVL;
  case Op_AbsF:
    assert(bt == T_FLOAT, "must be");
    return Op_AbsVF;
  case Op_AbsD:
    assert(bt == T_DOUBLE, "must be");
    return Op_AbsVD;
  case Op_NegF:
    assert(bt == T_FLOAT, "must be");
    return Op_NegVF;
  case Op_NegD:
    assert(bt == T_DOUBLE, "must be");
    return Op_NegVD;
  case Op_RoundDoubleMode:
    assert(bt == T_DOUBLE, "must be");
    return Op_RoundDoubleModeV;
  case Op_RotateLeft:
    assert(bt == T_LONG || bt == T_INT, "must be");
    return Op_RotateLeftV;
  case Op_RotateRight:
    assert(bt == T_LONG || bt == T_INT, "must be");
    return Op_RotateRightV;
  case Op_SqrtF:
    assert(bt == T_FLOAT, "must be");
    return Op_SqrtVF;
  case Op_SqrtD:
    assert(bt == T_DOUBLE, "must be");
    return Op_SqrtVD;
  case Op_PopCountI:
    if (bt == T_INT) {
      return Op_PopCountVI;
    }
    // Unimplemented for subword types since bit count changes
    // depending on size of lane (and sign bit).
    return 0;
  case Op_LShiftI:
    switch (bt) {
    case T_BOOLEAN:
    case T_BYTE:   return Op_LShiftVB;
    case T_CHAR:
    case T_SHORT:  return Op_LShiftVS;
    case T_INT:    return Op_LShiftVI;
      default:       ShouldNotReachHere(); return 0;
    }
  case Op_LShiftL:
    assert(bt == T_LONG, "must be");
    return Op_LShiftVL;
  case Op_RShiftI:
    switch (bt) {
    case T_BOOLEAN:return Op_URShiftVB; // boolean is unsigned value
    case T_CHAR:   return Op_URShiftVS; // char is unsigned value
    case T_BYTE:   return Op_RShiftVB;
    case T_SHORT:  return Op_RShiftVS;
    case T_INT:    return Op_RShiftVI;
    default:       ShouldNotReachHere(); return 0;
    }
  case Op_RShiftL:
    assert(bt == T_LONG, "must be");
    return Op_RShiftVL;
  case Op_URShiftI:
    switch (bt) {
    case T_BOOLEAN:return Op_URShiftVB;
    case T_CHAR:   return Op_URShiftVS;
    case T_BYTE:
    case T_SHORT:  return 0; // Vector logical right shift for signed short
                             // values produces incorrect Java result for
                             // negative data because java code should convert
                             // a short value into int value with sign
                             // extension before a shift.
    case T_INT:    return Op_URShiftVI;
    default:       ShouldNotReachHere(); return 0;
    }
  case Op_URShiftL:
    assert(bt == T_LONG, "must be");
    return Op_URShiftVL;
  case Op_AndI:
  case Op_AndL:
    return Op_AndV;
  case Op_OrI:
  case Op_OrL:
    return Op_OrV;
  case Op_XorI:
  case Op_XorL:
    return Op_XorV;
  case Op_MinF:
    assert(bt == T_FLOAT, "must be");
    return Op_MinV;
  case Op_MinD:
    assert(bt == T_DOUBLE, "must be");
    return Op_MinV;
  case Op_MaxF:
    assert(bt == T_FLOAT, "must be");
    return Op_MaxV;
  case Op_MaxD:
    assert(bt == T_DOUBLE, "must be");
    return Op_MaxV;

  case Op_LoadB:
  case Op_LoadUB:
  case Op_LoadUS:
  case Op_LoadS:
  case Op_LoadI:
  case Op_LoadL:
  case Op_LoadF:
  case Op_LoadD:
    return Op_LoadVector;

  case Op_StoreB:
  case Op_StoreC:
  case Op_StoreI:
  case Op_StoreL:
  case Op_StoreF:
  case Op_StoreD:
    return Op_StoreVector;
  case Op_MulAddS2I:
    return Op_MulAddVS2VI;

  default:
    return 0; // Unimplemented
  }
}

// Also used to check if the code generator
// supports the vector operation.
bool VectorNode::implemented(int opc, uint vlen, BasicType bt) {
  if (is_java_primitive(bt) &&
      (vlen > 1) && is_power_of_2(vlen) &&
      Matcher::vector_size_supported(bt, vlen)) {
    int vopc = VectorNode::opcode(opc, bt);
    // For rotate operation we will do a lazy de-generation into
    // OrV/LShiftV/URShiftV pattern if the target does not support
    // vector rotation instruction.
    if (vopc == Op_RotateLeftV || vopc == Op_RotateRightV) {
      return is_vector_rotate_supported(vopc, vlen, bt);
    }
    return vopc > 0 && Matcher::match_rule_supported_vector(vopc, vlen, bt);
  }
  return false;
}

bool VectorNode::is_type_transition_short_to_int(Node* n) {
  switch (n->Opcode()) {
  case Op_MulAddS2I:
    return true;
  }
  return false;
}

bool VectorNode::is_type_transition_to_int(Node* n) {
  return is_type_transition_short_to_int(n);
}

bool VectorNode::is_muladds2i(Node* n) {
  if (n->Opcode() == Op_MulAddS2I) {
    return true;
  }
  return false;
}

bool VectorNode::is_roundopD(Node* n) {
  if (n->Opcode() == Op_RoundDoubleMode) {
    return true;
  }
  return false;
}

bool VectorNode::is_scalar_rotate(Node* n) {
  if (n->Opcode() == Op_RotateLeft || n->Opcode() == Op_RotateRight) {
    return true;
  }
  return false;
}

bool VectorNode::is_vector_rotate_supported(int vopc, uint vlen, BasicType bt) {
  assert(vopc == Op_RotateLeftV || vopc == Op_RotateRightV, "wrong opcode");

  // If target defines vector rotation patterns then no
  // need for degeneration.
  if (Matcher::match_rule_supported_vector(vopc, vlen, bt)) {
    return true;
  }

  // Validate existence of nodes created in case of rotate degeneration.
  switch (bt) {
    case T_INT:
      return Matcher::match_rule_supported_vector(Op_OrV,       vlen, bt) &&
             Matcher::match_rule_supported_vector(Op_LShiftVI,  vlen, bt) &&
             Matcher::match_rule_supported_vector(Op_URShiftVI, vlen, bt);
    case T_LONG:
      return Matcher::match_rule_supported_vector(Op_OrV,       vlen, bt) &&
             Matcher::match_rule_supported_vector(Op_LShiftVL,  vlen, bt) &&
             Matcher::match_rule_supported_vector(Op_URShiftVL, vlen, bt);
    default:
      assert(false, "not supported: %s", type2name(bt));
      return false;
  }
}

bool VectorNode::is_shift(Node* n) {
  switch (n->Opcode()) {
  case Op_LShiftI:
  case Op_LShiftL:
  case Op_RShiftI:
  case Op_RShiftL:
  case Op_URShiftI:
  case Op_URShiftL:
    return true;
  default:
    return false;
  }
}

// Check if input is loop invariant vector.
bool VectorNode::is_invariant_vector(Node* n) {
  // Only Replicate vector nodes are loop invariant for now.
  switch (n->Opcode()) {
  case Op_ReplicateB:
  case Op_ReplicateS:
  case Op_ReplicateI:
  case Op_ReplicateL:
  case Op_ReplicateF:
  case Op_ReplicateD:
    return true;
  default:
    return false;
  }
}

// [Start, end) half-open range defining which operands are vectors
void VectorNode::vector_operands(Node* n, uint* start, uint* end) {
  switch (n->Opcode()) {
  case Op_LoadB:   case Op_LoadUB:
  case Op_LoadS:   case Op_LoadUS:
  case Op_LoadI:   case Op_LoadL:
  case Op_LoadF:   case Op_LoadD:
  case Op_LoadP:   case Op_LoadN:
    *start = 0;
    *end   = 0; // no vector operands
    break;
  case Op_StoreB:  case Op_StoreC:
  case Op_StoreI:  case Op_StoreL:
  case Op_StoreF:  case Op_StoreD:
  case Op_StoreP:  case Op_StoreN:
    *start = MemNode::ValueIn;
    *end   = MemNode::ValueIn + 1; // 1 vector operand
    break;
  case Op_LShiftI:  case Op_LShiftL:
  case Op_RShiftI:  case Op_RShiftL:
  case Op_URShiftI: case Op_URShiftL:
    *start = 1;
    *end   = 2; // 1 vector operand
    break;
  case Op_AddI: case Op_AddL: case Op_AddF: case Op_AddD:
  case Op_SubI: case Op_SubL: case Op_SubF: case Op_SubD:
  case Op_MulI: case Op_MulL: case Op_MulF: case Op_MulD:
  case Op_DivF: case Op_DivD:
  case Op_AndI: case Op_AndL:
  case Op_OrI:  case Op_OrL:
  case Op_XorI: case Op_XorL:
  case Op_MulAddS2I:
    *start = 1;
    *end   = 3; // 2 vector operands
    break;
  case Op_CMoveI:  case Op_CMoveL:  case Op_CMoveF:  case Op_CMoveD:
    *start = 2;
    *end   = n->req();
    break;
  case Op_FmaD:
  case Op_FmaF:
    *start = 1;
    *end   = 4; // 3 vector operands
    break;
  default:
    *start = 1;
    *end   = n->req(); // default is all operands
  }
}

// Return the vector version of a scalar operation node.
VectorNode* VectorNode::make(int opc, Node* n1, Node* n2, uint vlen, BasicType bt) {
  const TypeVect* vt = TypeVect::make(bt, vlen);
  int vopc = VectorNode::opcode(opc, bt);
  // This method should not be called for unimplemented vectors.
  guarantee(vopc > 0, "Vector for '%s' is not implemented", NodeClassNames[opc]);
  switch (vopc) {
  case Op_AddVB: return new AddVBNode(n1, n2, vt);
  case Op_AddVS: return new AddVSNode(n1, n2, vt);
  case Op_AddVI: return new AddVINode(n1, n2, vt);
  case Op_AddVL: return new AddVLNode(n1, n2, vt);
  case Op_AddVF: return new AddVFNode(n1, n2, vt);
  case Op_AddVD: return new AddVDNode(n1, n2, vt);

  case Op_SubVB: return new SubVBNode(n1, n2, vt);
  case Op_SubVS: return new SubVSNode(n1, n2, vt);
  case Op_SubVI: return new SubVINode(n1, n2, vt);
  case Op_SubVL: return new SubVLNode(n1, n2, vt);
  case Op_SubVF: return new SubVFNode(n1, n2, vt);
  case Op_SubVD: return new SubVDNode(n1, n2, vt);

  case Op_MulVB: return new MulVBNode(n1, n2, vt);
  case Op_MulVS: return new MulVSNode(n1, n2, vt);
  case Op_MulVI: return new MulVINode(n1, n2, vt);
  case Op_MulVL: return new MulVLNode(n1, n2, vt);
  case Op_MulVF: return new MulVFNode(n1, n2, vt);
  case Op_MulVD: return new MulVDNode(n1, n2, vt);

  case Op_DivVF: return new DivVFNode(n1, n2, vt);
  case Op_DivVD: return new DivVDNode(n1, n2, vt);

  case Op_AbsVB: return new AbsVBNode(n1, vt);
  case Op_AbsVS: return new AbsVSNode(n1, vt);
  case Op_AbsVI: return new AbsVINode(n1, vt);
  case Op_AbsVL: return new AbsVLNode(n1, vt);
  case Op_AbsVF: return new AbsVFNode(n1, vt);
  case Op_AbsVD: return new AbsVDNode(n1, vt);

  case Op_NegVF: return new NegVFNode(n1, vt);
  case Op_NegVD: return new NegVDNode(n1, vt);

  case Op_SqrtVF: return new SqrtVFNode(n1, vt);
  case Op_SqrtVD: return new SqrtVDNode(n1, vt);

  case Op_PopCountVI: return new PopCountVINode(n1, vt);
  case Op_RotateLeftV: return new RotateLeftVNode(n1, n2, vt);
  case Op_RotateRightV: return new RotateRightVNode(n1, n2, vt);

  case Op_LShiftVB: return new LShiftVBNode(n1, n2, vt);
  case Op_LShiftVS: return new LShiftVSNode(n1, n2, vt);
  case Op_LShiftVI: return new LShiftVINode(n1, n2, vt);
  case Op_LShiftVL: return new LShiftVLNode(n1, n2, vt);

  case Op_RShiftVB: return new RShiftVBNode(n1, n2, vt);
  case Op_RShiftVS: return new RShiftVSNode(n1, n2, vt);
  case Op_RShiftVI: return new RShiftVINode(n1, n2, vt);
  case Op_RShiftVL: return new RShiftVLNode(n1, n2, vt);

  case Op_URShiftVB: return new URShiftVBNode(n1, n2, vt);
  case Op_URShiftVS: return new URShiftVSNode(n1, n2, vt);
  case Op_URShiftVI: return new URShiftVINode(n1, n2, vt);
  case Op_URShiftVL: return new URShiftVLNode(n1, n2, vt);

  case Op_AndV: return new AndVNode(n1, n2, vt);
  case Op_OrV:  return new OrVNode (n1, n2, vt);
  case Op_XorV: return new XorVNode(n1, n2, vt);

  case Op_MinV: return new MinVNode(n1, n2, vt);
  case Op_MaxV: return new MaxVNode(n1, n2, vt);

  case Op_RoundDoubleModeV: return new RoundDoubleModeVNode(n1, n2, vt);

  case Op_MulAddVS2VI: return new MulAddVS2VINode(n1, n2, vt);
  default:
    fatal("Missed vector creation for '%s'", NodeClassNames[vopc]);
    return NULL;
  }
}

VectorNode* VectorNode::make(int opc, Node* n1, Node* n2, Node* n3, uint vlen, BasicType bt) {
  const TypeVect* vt = TypeVect::make(bt, vlen);
  int vopc = VectorNode::opcode(opc, bt);
  // This method should not be called for unimplemented vectors.
  guarantee(vopc > 0, "Vector for '%s' is not implemented", NodeClassNames[opc]);
  switch (vopc) {
  case Op_FmaVD: return new FmaVDNode(n1, n2, n3, vt);
  case Op_FmaVF: return new FmaVFNode(n1, n2, n3, vt);
  default:
    fatal("Missed vector creation for '%s'", NodeClassNames[vopc]);
    return NULL;
  }
}

// Scalar promotion
VectorNode* VectorNode::scalar2vector(Node* s, uint vlen, const Type* opd_t) {
  BasicType bt = opd_t->array_element_basic_type();
  const TypeVect* vt = opd_t->singleton() ? TypeVect::make(opd_t, vlen)
                                          : TypeVect::make(bt, vlen);
  switch (bt) {
  case T_BOOLEAN:
  case T_BYTE:
    return new ReplicateBNode(s, vt);
  case T_CHAR:
  case T_SHORT:
    return new ReplicateSNode(s, vt);
  case T_INT:
    return new ReplicateINode(s, vt);
  case T_LONG:
    return new ReplicateLNode(s, vt);
  case T_FLOAT:
    return new ReplicateFNode(s, vt);
  case T_DOUBLE:
    return new ReplicateDNode(s, vt);
  default:
    fatal("Type '%s' is not supported for vectors", type2name(bt));
    return NULL;
  }
}

VectorNode* VectorNode::shift_count(Node* shift, Node* cnt, uint vlen, BasicType bt) {
  assert(VectorNode::is_shift(shift), "sanity");
  // Match shift count type with shift vector type.
  const TypeVect* vt = TypeVect::make(bt, vlen);
  switch (shift->Opcode()) {
  case Op_LShiftI:
  case Op_LShiftL:
    return new LShiftCntVNode(cnt, vt);
  case Op_RShiftI:
  case Op_RShiftL:
  case Op_URShiftI:
  case Op_URShiftL:
    return new RShiftCntVNode(cnt, vt);
  default:
    fatal("Missed vector creation for '%s'", NodeClassNames[shift->Opcode()]);
    return NULL;
  }
}

bool VectorNode::is_vector_shift(int opc) {
  assert(opc > _last_machine_leaf && opc < _last_opcode, "invalid opcode");
  switch (opc) {
  case Op_LShiftVB:
  case Op_LShiftVS:
  case Op_LShiftVI:
  case Op_LShiftVL:
  case Op_RShiftVB:
  case Op_RShiftVS:
  case Op_RShiftVI:
  case Op_RShiftVL:
  case Op_URShiftVB:
  case Op_URShiftVS:
  case Op_URShiftVI:
  case Op_URShiftVL:
    return true;
  default:
    return false;
  }
}

bool VectorNode::is_vector_shift_count(int opc) {
  assert(opc > _last_machine_leaf && opc < _last_opcode, "invalid opcode");
  switch (opc) {
  case Op_RShiftCntV:
  case Op_LShiftCntV:
    return true;
  default:
    return false;
  }
}

static bool is_con_M1(Node* n) {
  if (n->is_Con()) {
    const Type* t = n->bottom_type();
    if (t->isa_int() && t->is_int()->get_con() == -1) {
      return true;
    }
    if (t->isa_long() && t->is_long()->get_con() == -1) {
      return true;
    }
  }
  return false;
}

bool VectorNode::is_all_ones_vector(Node* n) {
  switch (n->Opcode()) {
  case Op_ReplicateB:
  case Op_ReplicateS:
  case Op_ReplicateI:
  case Op_ReplicateL:
    return is_con_M1(n->in(1));
  default:
    return false;
  }
}

bool VectorNode::is_vector_bitwise_not_pattern(Node* n) {
  if (n->Opcode() == Op_XorV) {
    return is_all_ones_vector(n->in(1)) ||
           is_all_ones_vector(n->in(2));
  }
  return false;
}

// Return initial Pack node. Additional operands added with add_opd() calls.
PackNode* PackNode::make(Node* s, uint vlen, BasicType bt) {
  const TypeVect* vt = TypeVect::make(bt, vlen);
  switch (bt) {
  case T_BOOLEAN:
  case T_BYTE:
    return new PackBNode(s, vt);
  case T_CHAR:
  case T_SHORT:
    return new PackSNode(s, vt);
  case T_INT:
    return new PackINode(s, vt);
  case T_LONG:
    return new PackLNode(s, vt);
  case T_FLOAT:
    return new PackFNode(s, vt);
  case T_DOUBLE:
    return new PackDNode(s, vt);
  default:
    fatal("Type '%s' is not supported for vectors", type2name(bt));
    return NULL;
  }
}

// Create a binary tree form for Packs. [lo, hi) (half-open) range
PackNode* PackNode::binary_tree_pack(int lo, int hi) {
  int ct = hi - lo;
  assert(is_power_of_2(ct), "power of 2");
  if (ct == 2) {
    PackNode* pk = PackNode::make(in(lo), 2, vect_type()->element_basic_type());
    pk->add_opd(in(lo+1));
    return pk;
  } else {
    int mid = lo + ct/2;
    PackNode* n1 = binary_tree_pack(lo,  mid);
    PackNode* n2 = binary_tree_pack(mid, hi );

    BasicType bt = n1->vect_type()->element_basic_type();
    assert(bt == n2->vect_type()->element_basic_type(), "should be the same");
    switch (bt) {
    case T_BOOLEAN:
    case T_BYTE:
      return new PackSNode(n1, n2, TypeVect::make(T_SHORT, 2));
    case T_CHAR:
    case T_SHORT:
      return new PackINode(n1, n2, TypeVect::make(T_INT, 2));
    case T_INT:
      return new PackLNode(n1, n2, TypeVect::make(T_LONG, 2));
    case T_LONG:
      return new Pack2LNode(n1, n2, TypeVect::make(T_LONG, 2));
    case T_FLOAT:
      return new PackDNode(n1, n2, TypeVect::make(T_DOUBLE, 2));
    case T_DOUBLE:
      return new Pack2DNode(n1, n2, TypeVect::make(T_DOUBLE, 2));
    default:
      fatal("Type '%s' is not supported for vectors", type2name(bt));
      return NULL;
    }
  }
}

// Return the vector version of a scalar load node.
LoadVectorNode* LoadVectorNode::make(int opc, Node* ctl, Node* mem,
                                     Node* adr, const TypePtr* atyp,
                                     uint vlen, BasicType bt,
                                     ControlDependency control_dependency) {
  const TypeVect* vt = TypeVect::make(bt, vlen);
  return new LoadVectorNode(ctl, mem, adr, atyp, vt, control_dependency);
}

// Return the vector version of a scalar store node.
StoreVectorNode* StoreVectorNode::make(int opc, Node* ctl, Node* mem,
                                       Node* adr, const TypePtr* atyp, Node* val,
                                       uint vlen) {
  return new StoreVectorNode(ctl, mem, adr, atyp, val);
}

// Extract a scalar element of vector.
Node* ExtractNode::make(Node* v, uint position, BasicType bt) {
  assert((int)position < Matcher::max_vector_size(bt), "pos in range");
  ConINode* pos = ConINode::make((int)position);
  switch (bt) {
  case T_BOOLEAN:
    return new ExtractUBNode(v, pos);
  case T_BYTE:
    return new ExtractBNode(v, pos);
  case T_CHAR:
    return new ExtractCNode(v, pos);
  case T_SHORT:
    return new ExtractSNode(v, pos);
  case T_INT:
    return new ExtractINode(v, pos);
  case T_LONG:
    return new ExtractLNode(v, pos);
  case T_FLOAT:
    return new ExtractFNode(v, pos);
  case T_DOUBLE:
    return new ExtractDNode(v, pos);
  default:
    fatal("Type '%s' is not supported for vectors", type2name(bt));
    return NULL;
  }
}

int ReductionNode::opcode(int opc, BasicType bt) {
  int vopc = opc;
  switch (opc) {
    case Op_AddI:
      assert(bt == T_INT, "must be");
      vopc = Op_AddReductionVI;
      break;
    case Op_AddL:
      assert(bt == T_LONG, "must be");
      vopc = Op_AddReductionVL;
      break;
    case Op_AddF:
      assert(bt == T_FLOAT, "must be");
      vopc = Op_AddReductionVF;
      break;
    case Op_AddD:
      assert(bt == T_DOUBLE, "must be");
      vopc = Op_AddReductionVD;
      break;
    case Op_MulI:
      assert(bt == T_INT, "must be");
      vopc = Op_MulReductionVI;
      break;
    case Op_MulL:
      assert(bt == T_LONG, "must be");
      vopc = Op_MulReductionVL;
      break;
    case Op_MulF:
      assert(bt == T_FLOAT, "must be");
      vopc = Op_MulReductionVF;
      break;
    case Op_MulD:
      assert(bt == T_DOUBLE, "must be");
      vopc = Op_MulReductionVD;
      break;
    case Op_MinF:
      assert(bt == T_FLOAT, "must be");
      vopc = Op_MinReductionV;
      break;
    case Op_MinD:
      assert(bt == T_DOUBLE, "must be");
      vopc = Op_MinReductionV;
      break;
    case Op_MaxF:
      assert(bt == T_FLOAT, "must be");
      vopc = Op_MaxReductionV;
      break;
    case Op_MaxD:
      assert(bt == T_DOUBLE, "must be");
      vopc = Op_MaxReductionV;
      break;
    case Op_AndI:
      assert(bt == T_INT, "must be");
      vopc = Op_AndReductionV;
      break;
    case Op_AndL:
      assert(bt == T_LONG, "must be");
      vopc = Op_AndReductionV;
      break;
    case Op_OrI:
      assert(bt == T_INT, "must be");
      vopc = Op_OrReductionV;
      break;
    case Op_OrL:
      assert(bt == T_LONG, "must be");
      vopc = Op_OrReductionV;
      break;
    case Op_XorI:
      assert(bt == T_INT, "must be");
      vopc = Op_XorReductionV;
      break;
    case Op_XorL:
      assert(bt == T_LONG, "must be");
      vopc = Op_XorReductionV;
      break;
    default:
      break;
  }
  return vopc;
}

// Return the appropriate reduction node.
ReductionNode* ReductionNode::make(int opc, Node *ctrl, Node* n1, Node* n2, BasicType bt) {

  int vopc = opcode(opc, bt);

  // This method should not be called for unimplemented vectors.
  guarantee(vopc != opc, "Vector for '%s' is not implemented", NodeClassNames[opc]);

  switch (vopc) {
  case Op_AddReductionVI: return new AddReductionVINode(ctrl, n1, n2);
  case Op_AddReductionVL: return new AddReductionVLNode(ctrl, n1, n2);
  case Op_AddReductionVF: return new AddReductionVFNode(ctrl, n1, n2);
  case Op_AddReductionVD: return new AddReductionVDNode(ctrl, n1, n2);
  case Op_MulReductionVI: return new MulReductionVINode(ctrl, n1, n2);
  case Op_MulReductionVL: return new MulReductionVLNode(ctrl, n1, n2);
  case Op_MulReductionVF: return new MulReductionVFNode(ctrl, n1, n2);
  case Op_MulReductionVD: return new MulReductionVDNode(ctrl, n1, n2);
  case Op_MinReductionV:  return new MinReductionVNode(ctrl, n1, n2);
  case Op_MaxReductionV:  return new MaxReductionVNode(ctrl, n1, n2);
  case Op_AndReductionV:  return new AndReductionVNode(ctrl, n1, n2);
  case Op_OrReductionV:   return new OrReductionVNode(ctrl, n1, n2);
  case Op_XorReductionV:  return new XorReductionVNode(ctrl, n1, n2);
  default:
    fatal("Missed vector creation for '%s'", NodeClassNames[vopc]);
    return NULL;
  }
}

bool ReductionNode::implemented(int opc, uint vlen, BasicType bt) {
  if (is_java_primitive(bt) &&
      (vlen > 1) && is_power_of_2(vlen) &&
      Matcher::vector_size_supported(bt, vlen)) {
    int vopc = ReductionNode::opcode(opc, bt);
    return vopc != opc && Matcher::match_rule_supported(vopc);
  }
  return false;
}

MacroLogicVNode* MacroLogicVNode::make(PhaseGVN& gvn, Node* in1, Node* in2, Node* in3,
                                      uint truth_table, const TypeVect* vt) {
  assert(truth_table <= 0xFF, "invalid");
  assert(in1->bottom_type()->is_vect()->length_in_bytes() == vt->length_in_bytes(), "mismatch");
  assert(in2->bottom_type()->is_vect()->length_in_bytes() == vt->length_in_bytes(), "mismatch");
  assert(in3->bottom_type()->is_vect()->length_in_bytes() == vt->length_in_bytes(), "mismatch");
  Node* fn = gvn.intcon(truth_table);
  return new MacroLogicVNode(in1, in2, in3, fn, vt);
}

Node* VectorNode::degenerate_vector_rotate(Node* src, Node* cnt, bool is_rotate_left,
                                           int vlen, BasicType bt, PhaseGVN* phase) {
  int shiftLOpc;
  int shiftROpc;
  Node* shiftLCnt = NULL;
  Node* shiftRCnt = NULL;
  const TypeVect* vt = TypeVect::make(bt, vlen);

  // Compute shift values for right rotation and
  // later swap them in case of left rotation.
  if (cnt->is_Con()) {
    // Constant shift case.
    if (bt == T_INT) {
      int shift = cnt->get_int() & 31;
      shiftRCnt = phase->intcon(shift);
      shiftLCnt = phase->intcon(32 - shift);
      shiftLOpc = Op_LShiftI;
      shiftROpc = Op_URShiftI;
    } else {
      int shift = cnt->get_int() & 63;
      shiftRCnt = phase->intcon(shift);
      shiftLCnt = phase->intcon(64 - shift);
      shiftLOpc = Op_LShiftL;
      shiftROpc = Op_URShiftL;
    }
  } else {
    // Variable shift case.
    assert(VectorNode::is_invariant_vector(cnt), "Broadcast expected");
    cnt = cnt->in(1);
    if (bt == T_INT) {
      shiftRCnt = phase->transform(new AndINode(cnt, phase->intcon(31)));
      shiftLCnt = phase->transform(new SubINode(phase->intcon(32), shiftRCnt));
      shiftLOpc = Op_LShiftI;
      shiftROpc = Op_URShiftI;
    } else {
      assert(cnt->Opcode() == Op_ConvI2L, "ConvI2L expected");
      cnt = cnt->in(1);
      shiftRCnt = phase->transform(new AndINode(cnt, phase->intcon(63)));
      shiftLCnt = phase->transform(new SubINode(phase->intcon(64), shiftRCnt));
      shiftLOpc = Op_LShiftL;
      shiftROpc = Op_URShiftL;
    }
  }

  // Swap the computed left and right shift counts.
  if (is_rotate_left) {
    swap(shiftRCnt,shiftLCnt);
  }

  shiftLCnt = phase->transform(new LShiftCntVNode(shiftLCnt, vt));
  shiftRCnt = phase->transform(new RShiftCntVNode(shiftRCnt, vt));

  return new OrVNode(phase->transform(VectorNode::make(shiftLOpc, src, shiftLCnt, vlen, bt)),
                     phase->transform(VectorNode::make(shiftROpc, src, shiftRCnt, vlen, bt)),
                     vt);
}

Node* RotateLeftVNode::Ideal(PhaseGVN* phase, bool can_reshape) {
  int vlen = length();
  BasicType bt = vect_type()->element_basic_type();
  if (!Matcher::match_rule_supported_vector(Op_RotateLeftV, vlen, bt)) {
    return VectorNode::degenerate_vector_rotate(in(1), in(2), true, vlen, bt, phase);
  }
  return NULL;
}

Node* RotateRightVNode::Ideal(PhaseGVN* phase, bool can_reshape) {
  int vlen = length();
  BasicType bt = vect_type()->element_basic_type();
  if (!Matcher::match_rule_supported_vector(Op_RotateRightV, vlen, bt)) {
    return VectorNode::degenerate_vector_rotate(in(1), in(2), false, vlen, bt, phase);
  }
  return NULL;
}

Node* OrVNode::Ideal(PhaseGVN* phase, bool can_reshape) {
  int lopcode = in(1)->Opcode();
  int ropcode = in(2)->Opcode();
  const TypeVect* vt = bottom_type()->is_vect();
  int vec_len = vt->length();
  BasicType bt = vt->element_basic_type();

  // Vector Rotate operations inferencing, this will be useful when vector
  // operations are created via non-SLP route i.e. (VectorAPI).
  if (Matcher::match_rule_supported_vector(Op_RotateLeftV, vec_len, bt) &&
      ((ropcode == Op_LShiftVI && lopcode == Op_URShiftVI) ||
       (ropcode == Op_LShiftVL && lopcode == Op_URShiftVL)) &&
      in(1)->in(1) == in(2)->in(1)) {
    assert(Op_RShiftCntV == in(1)->in(2)->Opcode(), "LShiftCntV operand expected");
    assert(Op_LShiftCntV == in(2)->in(2)->Opcode(), "RShiftCntV operand expected");
    Node* lshift = in(1)->in(2)->in(1);
    Node* rshift = in(2)->in(2)->in(1);
    int mod_val = bt == T_LONG ? 64 : 32;
    int shift_mask = bt == T_LONG ? 0x3F : 0x1F;
    // val >> norm_con_shift | val << (32 - norm_con_shift) => rotate_right val ,
    // norm_con_shift
    if (lshift->is_Con() && rshift->is_Con() &&
        ((lshift->get_int() & shift_mask) ==
         (mod_val - (rshift->get_int() & shift_mask)))) {
      return new RotateRightVNode(
          in(1)->in(1), phase->intcon(lshift->get_int() & shift_mask), vt);
    }
    if (lshift->Opcode() == Op_AndI && rshift->Opcode() == Op_AndI &&
        lshift->in(2)->is_Con() && rshift->in(2)->is_Con() &&
        lshift->in(2)->get_int() == (mod_val - 1) &&
        rshift->in(2)->get_int() == (mod_val - 1)) {
      lshift = lshift->in(1);
      rshift = rshift->in(1);
      // val << var_shift | val >> (0/32 - var_shift) => rotate_left val ,
      // var_shift
      if (lshift->Opcode() == Op_SubI && lshift->in(2) == rshift &&
          lshift->in(1)->is_Con() &&
          (lshift->in(1)->get_int() == 0 ||
           lshift->in(1)->get_int() == mod_val)) {
        Node* rotate_cnt = phase->transform(new ReplicateINode(rshift, vt));
        return new RotateLeftVNode(in(1)->in(1), rotate_cnt, vt);
      }
    }
  }

  if (Matcher::match_rule_supported_vector(Op_RotateRightV, vec_len, bt) &&
      ((ropcode == Op_URShiftVI && lopcode == Op_LShiftVI) ||
       (ropcode == Op_URShiftVL && lopcode == Op_LShiftVL)) &&
      in(1)->in(1) == in(2)->in(1)) {
    assert(Op_LShiftCntV == in(1)->in(2)->Opcode(), "RShiftCntV operand expected");
    assert(Op_RShiftCntV == in(2)->in(2)->Opcode(), "LShiftCntV operand expected");
    Node* rshift = in(1)->in(2)->in(1);
    Node* lshift = in(2)->in(2)->in(1);
    int mod_val = bt == T_LONG ? 64 : 32;
    int shift_mask = bt == T_LONG ? 0x3F : 0x1F;
    // val << norm_con_shift | val >> (32 - norm_con_shift) => rotate_left val
    // , norm_con_shift
    if (rshift->is_Con() && lshift->is_Con() &&
        ((rshift->get_int() & shift_mask) ==
         (mod_val - (lshift->get_int() & shift_mask)))) {
      return new RotateLeftVNode(
          in(1)->in(1), phase->intcon(rshift->get_int() & shift_mask), vt);
    }
    if (lshift->Opcode() == Op_AndI && rshift->Opcode() == Op_AndI &&
        lshift->in(2)->is_Con() && rshift->in(2)->is_Con() &&
        rshift->in(2)->get_int() == (mod_val - 1) &&
        lshift->in(2)->get_int() == (mod_val - 1)) {
      rshift = rshift->in(1);
      lshift = lshift->in(1);
      // val >> var_shift | val << (0/32 - var_shift) => rotate_right val ,
      // var_shift
      if (rshift->Opcode() == Op_SubI && rshift->in(2) == lshift &&
          rshift->in(1)->is_Con() &&
          (rshift->in(1)->get_int() == 0 ||
           rshift->in(1)->get_int() == mod_val)) {
        Node* rotate_cnt = phase->transform(new ReplicateINode(lshift, vt));
        return new RotateRightVNode(in(1)->in(1), rotate_cnt, vt);
      }
    }
  }
  return NULL;
}
