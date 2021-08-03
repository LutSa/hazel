type t =
  | OpHole(closure)
  | TypeErr(closure, t)
  | Var(option(closure), VarId.t)
  | Tuple(list(t))
  | Ap(t, t)
  | BoolLit(bool)
  | IntLit(int)
  | FloatLit(float)
  | Keyword(closure, ExpandingKeyword.t)
  | InvalidText(closure, string)
  | BoolBin(BinBoolOp.t, t, t)
  | IntBin(BinIntOp.t, t, t)
  | FloatBin(BinFloatOp.t, t, t)
  // TODO remove Type.t now that patterns have type anns?
  | Fun(DTerm_pat.t, Type.t, t)
  | Fix(VarId.t, Type.t, t)
  | Let(DTerm_pat.t, t, t)
  | ListNil(Type.t)
  | Cons(t, t)
  | Inj(Type.t, InjSide.t, t)
  | Case(option(closure), t, list(rules), int)
  | Cast(t, HTyp.t, HTyp.t)
  | FailedCast(t, HTyp.t, HTyp.t)
  | InvalidOperation(t, InvalidOperationError.t)
and rule = (DTerm_pat.t, t);
and closure = (MetaVar.t, MetaVarInst.t, VarMap.t_(t));
