open OptUtil.Syntax;
open Assistant_common;

type assistant_action_categories =
  | InsertLit
  | InsertVar
  | InsertApp
  | InsertConstructor;

type assistant_action = {
  category: assistant_action_categories,
  text: string,
  action: Action.t,
  result: UHExp.t,
  res_ty: HTyp.t,
};

let wrap = operand =>
  Seq.mk(operand, []) |> UHExp.mk_OpSeq |> UHExp.Block.wrap';

let mk_hole = u_gen => fst(UHExp.new_EmptyHole(u_gen));

let mk_var_uhexp = name => wrap(UHExp.var(name));

let mk_bool_lit_uhexp = b => wrap(UHExp.boollit(b));

let mk_list_nil_uhexp = wrap(UHExp.listnil());

let wrap_space = (operator, seq) =>
  Seq.S(operator, A(Operators_Exp.Space, seq));

let rec mk_ap_seq_holes =
        (f_ty: HTyp.t, hole_ty: HTyp.t)
        : option(Seq.t(UHExp.operand, UHExp.operator)) => {
  switch (f_ty) {
  | Arrow(_, output_ty) =>
    if (HTyp.consistent(output_ty, hole_ty)) {
      Some(S(EmptyHole(0), E));
    } else {
      let+ affix = mk_ap_seq_holes(output_ty, hole_ty);
      wrap_space(UHExp.EmptyHole(0), affix);
    }
  | _ => None
  };
};

let mk_ap =
    ({expected_ty, ctx, u_gen, _}: cursor_info_pro, f: Var.t, f_ty: HTyp.t)
    : option(UHExp.t) => {
  let+ inner_seq = mk_ap_seq_holes(f_ty, expected_ty);
  wrap_space(UHExp.var(f), inner_seq)
  |> UHExp.mk_OpSeq
  |> UHExp.Block.wrap'
  |> Statics_Exp.syn_fix_holes(ctx, u_gen, ~renumber_empty_holes=true)
  |> (((x, _, _)) => x);
};

let mk_var_action =
    (_: cursor_info_pro, name: string, ty: HTyp.t): assistant_action => {
  let e = mk_var_uhexp(name);
  {
    category: InsertVar,
    action: ReplaceAtCursor(UHExp.var(name)),
    text: name,
    result: e,
    res_ty: ty,
  };
};

let mk_app_action =
    (cursor: cursor_info_pro, name: string, ty: HTyp.t): assistant_action => {
  let e =
    mk_ap(cursor, name, ty) |> OptUtil.get(_ => failwith("mk_app_action"));
  {
    category: InsertApp,
    text: name,
    action: ReplaceAtCursor(UHExp.Parenthesized(e)),
    res_ty: ty,
    result: e,
  };
};

let mk_bool_lit_action = (b: bool): assistant_action => {
  {
    category: InsertLit,
    text: string_of_bool(b),
    action: ReplaceAtCursor(UHExp.boollit(b)),
    res_ty: HTyp.Bool,
    result: mk_bool_lit_uhexp(b),
  };
};

let mk_nil_list_action: assistant_action = {
  {
    category: InsertLit,
    text: "[]",
    action: ReplaceAtCursor(UHExp.listnil()),
    res_ty: HTyp.Bool,
    result: mk_list_nil_uhexp,
  };
};

let mk_inj_action =
    ({u_gen, _}: cursor_info_pro, side: InjSide.t): assistant_action => {
  let operand = UHExp.inj(side, wrap(mk_hole(u_gen)));
  let uhexp = Seq.mk(operand, []) |> UHExp.mk_OpSeq |> UHExp.Block.wrap';
  {
    category: InsertConstructor,
    text: "inj" ++ InjSide.to_string(side) ++ "",
    // TODO: reconcile visual presentation ie brackets
    action: ReplaceAtCursor(operand),
    res_ty: HTyp.Bool,
    result: uhexp,
  };
};

let mk_case_action = ({u_gen, _}: cursor_info_pro): assistant_action => {
  let rule =
    UHExp.Rule(
      UHPat.new_EmptyHole(u_gen + 1)
      |> (((o, _)) => o |> (op => Seq.mk(op, []) |> UHPat.mk_OpSeq)),
      wrap(mk_hole(u_gen + 2)),
    );
  let operand = UHExp.case(wrap(mk_hole(u_gen)), [rule]);
  let uhexp = Seq.mk(operand, []) |> UHExp.mk_OpSeq |> UHExp.Block.wrap';
  {
    category: InsertConstructor, // TODO(andrew): new category
    text: "case",
    action: ReplaceAtCursor(operand),
    res_ty: HTyp.Hole,
    result: uhexp,
  };
};

let compute_lit_actions = ({expected_ty, mode, _} as cursor: cursor_info_pro) => {
  switch (mode) {
  // TODO(andrew): move these somewhere more sensible
  | Synthetic => [
      mk_inj_action(cursor, L),
      mk_inj_action(cursor, R),
      mk_case_action(cursor),
    ]
  | Analytic =>
    switch (expected_ty) {
    | Bool => [mk_bool_lit_action(true), mk_bool_lit_action(false)]
    | List(_) => [mk_nil_list_action]
    | _ => []
    }
  | UnknownMode => []
  };
};

let compute_var_actions =
    ({ctx, expected_ty, mode, _} as cursor: cursor_info_pro) => {
  switch (mode) {
  | Synthetic =>
    extract_vars(ctx, Hole)
    |> List.map(((name, var_ty)) => mk_var_action(cursor, name, var_ty))
  | Analytic =>
    extract_vars(ctx, expected_ty)
    |> List.map(((name, var_ty)) => mk_var_action(cursor, name, var_ty))
  | UnknownMode => []
  };
};

let compute_app_actions =
    ({ctx, expected_ty, mode, _} as cursor: cursor_info_pro) => {
  switch (mode) {
  | Synthetic
  | UnknownMode => []
  | Analytic =>
    fun_vars(ctx, expected_ty)
    |> List.map(((name, f_ty)) => mk_app_action(cursor, name, f_ty))
  };
};