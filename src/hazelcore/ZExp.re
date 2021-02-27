open Sexplib.Std;

[@deriving sexp]
type t = zblock
and zblock = ZList.t(zline, UHExp.line)
and zline =
  | CursorL(CursorPosition.t, UHExp.line)
  | ExpLineZ(zopseq)
  | LetLineZP(AbbrevErrStatus.t, ZPat.t, UHExp.t)
  | LetLineZE(AbbrevErrStatus.t, UHPat.t, t)
  | LivelitDefLineZExpansionType({
      name: (VarErrStatus.t, string),
      expansion_type: ZTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZCaptures({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZModelType({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: ZTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZActionType({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: ZTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZInit({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZUpdate({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZView({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: t,
      shape: UHExp.t,
      expand: UHExp.t,
    })
  | LivelitDefLineZShape({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: t,
      expand: UHExp.t,
    })
  | LivelitDefLineZExpand({
      name: (VarErrStatus.t, string),
      expansion_type: UHTyp.t,
      captures: UHExp.t,
      model_type: UHTyp.t,
      action_type: UHTyp.t,
      init: UHExp.t,
      update: UHExp.t,
      view: UHExp.t,
      shape: UHExp.t,
      expand: t,
    })
and zopseq = ZOpSeq.t(UHExp.operand, UHExp.operator, zoperand, zoperator)
and zoperand =
  | CursorE(CursorPosition.t, UHExp.operand)
  | ParenthesizedZ(t)
  | LamZP(ErrStatus.t, ZPat.t, UHExp.t)
  | LamZE(ErrStatus.t, UHPat.t, t)
  | InjZ(ErrStatus.t, InjSide.t, t)
  | CaseZE(CaseErrStatus.t, t, list(UHExp.rule))
  | CaseZR(CaseErrStatus.t, UHExp.t, zrules)
  | SubscriptZE1(ErrStatus.t, t, UHExp.t, UHExp.t)
  | SubscriptZE2(ErrStatus.t, UHExp.t, t, UHExp.t)
  | SubscriptZE3(ErrStatus.t, UHExp.t, UHExp.t, t)
  | ApLivelitZ(
      MetaVar.t,
      ErrStatus.t,
      LivelitName.t,
      LivelitName.t,
      SerializedModel.t,
      ZSpliceInfo.t(UHExp.t, t),
    )
and zoperator = (CursorPosition.t, UHExp.operator)
and zrules = ZList.t(zrule, UHExp.rule)
and zrule =
  | CursorR(CursorPosition.t, UHExp.rule)
  | RuleZP(ZPat.t, UHExp.t)
  | RuleZE(UHPat.t, t);

type operand_surround = Seq.operand_surround(UHExp.operand, UHExp.operator);
type operator_surround = Seq.operator_surround(UHExp.operand, UHExp.operator);
type zseq = ZSeq.t(UHExp.operand, UHExp.operator, zoperand, zoperator);

let rec is_opseq =
        (ze: t): option(ZSeq.t('operand, 'operator, 'zoperand, 'zoperator)) =>
  ze |> is_opseq_zblock
and is_opseq_zblock =
    ((_, zline, _): zblock)
    : option(ZSeq.t('operand, 'operator, 'zoperand, 'zoperator)) =>
  zline |> is_opseq_zline
and is_opseq_zline =
  fun
  | CursorL(_) => None
  | ExpLineZ(zopseq) => zopseq |> is_opseq_zopseq
  | LetLineZP(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_) => None
  | LivelitDefLineZCaptures({captures: zdef, _})
  | LivelitDefLineZInit({init: zdef, _})
  | LivelitDefLineZUpdate({update: zdef, _})
  | LivelitDefLineZView({view: zdef, _})
  | LivelitDefLineZShape({shape: zdef, _})
  | LivelitDefLineZExpand({expand: zdef, _})
  | LetLineZE(_, _, zdef) => zdef |> is_opseq
and is_opseq_zopseq =
  fun
  | ZOpSeq(_, ZOperand(_, (prefix, _)) as zseq) =>
    switch (prefix) {
    | Seq.A(Space, _) => Some(zseq)
    | _ => None
    }
  | ZOpSeq(_, ZOperator(_, _) as zseq) => Some(zseq);

let rec find_zoperand = (ze: t): option(zoperand) =>
  ze |> find_zoperand_zblock
and find_zoperand_zblock = ((_, zline, _): zblock): option(zoperand) =>
  zline |> find_zoperand_zline
and find_zoperand_zline =
  fun
  | CursorL(_) => None
  | ExpLineZ(zopseq) => zopseq |> find_zoperand_zopseq
  | LetLineZP(_) => None
  | LetLineZE(_, _, zdef) => zdef |> find_zoperand
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_) => None
  | LivelitDefLineZCaptures({captures: zdef, _})
  | LivelitDefLineZInit({init: zdef, _})
  | LivelitDefLineZUpdate({update: zdef, _})
  | LivelitDefLineZView({view: zdef, _})
  | LivelitDefLineZShape({shape: zdef, _})
  | LivelitDefLineZExpand({expand: zdef, _}) => zdef |> find_zoperand
and find_zoperand_zopseq =
  fun
  | ZOpSeq(_, ZOperand(zoperand, _)) => zoperand |> find_zoperand_zoperand
  | ZOpSeq(_, ZOperator(_)) => None
and find_zoperand_zoperator =
  fun
  | _ => None
and find_zoperand_zoperand =
  fun
  | zoperand => Some(zoperand);

let line_can_be_swapped = (line: zline): bool =>
  switch (line) {
  | CursorL(_)
  | LetLineZP(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_)
  | ExpLineZ(ZOpSeq(_, ZOperator(_)))
  | ExpLineZ(ZOpSeq(_, ZOperand(CursorE(_), _)))
  | ExpLineZ(ZOpSeq(_, ZOperand(LamZP(_), _))) => true
  | LetLineZE(_)
  | LivelitDefLineZCaptures(_)
  | LivelitDefLineZInit(_)
  | LivelitDefLineZUpdate(_)
  | LivelitDefLineZView(_)
  | LivelitDefLineZShape(_)
  | LivelitDefLineZExpand(_)
  | ExpLineZ(
      ZOpSeq(
        _,
        ZOperand(
          LamZE(_) | InjZ(_) | CaseZE(_) | CaseZR(_) | ParenthesizedZ(_) |
          SubscriptZE1(_) |
          SubscriptZE2(_) |
          SubscriptZE3(_) |
          ApLivelitZ(_),
          _,
        ),
      ),
    ) =>
    false
  };
let valid_cursors_line = (line: UHExp.line): list(CursorPosition.t) =>
  switch (line) {
  | CommentLine(comment) =>
    CursorPosition.[OnDelim(0, Before), OnDelim(0, After)]
    @ (
      ListUtil.range(String.length(comment) + 1)
      |> List.map(i => CursorPosition.OnText(i))
    )
  | ExpLine(_) => []
  | EmptyLine => [OnText(0)]
  | LetLine(_) =>
    CursorPosition.delim_cursors_k(0)
    @ CursorPosition.delim_cursors_k(1)
    @ CursorPosition.delim_cursors_k(2)
  | LivelitDefLine({name: (_, name), _}) =>
    // livelit <name> at <expansion_type> { <body> + }
    CursorPosition.delim_cursors_k(0)  // "livelit"
    @ CursorPosition.text_cursors(String.length(name))  // <name>
    @ CursorPosition.delim_cursors_k(1)  // "at"
    @ CursorPosition.delim_cursors_k(2)  // "{"
    @ CursorPosition.delim_cursors_k(3) // "}"
  };
let valid_cursors_operator: UHExp.operator => list(CursorPosition.t) =
  fun
  | _ => [OnOp(Before), OnOp(After)];
let valid_cursors_operand: UHExp.operand => list(CursorPosition.t) =
  fun
  /* outer nodes - delimiter */
  | EmptyHole(_)
  | ListNil(_) => CursorPosition.delim_cursors(1)
  /* outer nodes - text */
  | InvalidText(_, t) => CursorPosition.text_cursors(String.length(t))
  | Var(_, _, x) => CursorPosition.text_cursors(Var.length(x))
  | IntLit(_, n) => CursorPosition.text_cursors(String.length(n))
  | FloatLit(_, f) => CursorPosition.text_cursors(String.length(f))
  | BoolLit(_, b) => CursorPosition.text_cursors(b ? 4 : 5)
  | StringLit(_, s) =>
    List.append(
      CursorPosition.delim_cursors(2),
      CursorPosition.text_cursors(String.length(s)),
    )
  /* inner nodes */
  | Lam(_, _, _) => {
      CursorPosition.delim_cursors_k(0)
      @ CursorPosition.delim_cursors_k(1)
      @ CursorPosition.delim_cursors_k(2);
    }
  | Inj(_) => CursorPosition.delim_cursors(2)
  | Case(_) => CursorPosition.delim_cursors(2)
  | Parenthesized(_) => CursorPosition.delim_cursors(2)
  | Subscript(_) => CursorPosition.delim_cursors(3)
  | ApLivelit(_, _, _, name, _, _) =>
    CursorPosition.text_cursors(LivelitName.length(name))
  | FreeLivelit(_, name) =>
    CursorPosition.text_cursors(LivelitName.length(name));

let valid_cursors_rule = (_: UHExp.rule): list(CursorPosition.t) =>
  CursorPosition.delim_cursors(2);

let is_valid_cursor_line = (cursor: CursorPosition.t, line: UHExp.line): bool =>
  valid_cursors_line(line) |> List.mem(cursor);
let is_valid_cursor_operand =
    (cursor: CursorPosition.t, operand: UHExp.operand): bool =>
  valid_cursors_operand(operand) |> List.mem(cursor);
let is_valid_cursor_operator =
    (cursor: CursorPosition.t, operator: UHExp.operator): bool =>
  valid_cursors_operator(operator) |> List.mem(cursor);
let is_valid_cursor_rule = (cursor: CursorPosition.t, rule: UHExp.rule): bool =>
  valid_cursors_rule(rule) |> List.mem(cursor);

module ZLine = {
  let force_get_zopseq =
    fun
    | CursorL(_)
    | LetLineZP(_)
    | LetLineZE(_)
    | LivelitDefLineZExpansionType(_)
    | LivelitDefLineZCaptures(_)
    | LivelitDefLineZModelType(_)
    | LivelitDefLineZActionType(_)
    | LivelitDefLineZInit(_)
    | LivelitDefLineZUpdate(_)
    | LivelitDefLineZView(_)
    | LivelitDefLineZShape(_)
    | LivelitDefLineZExpand(_) =>
      failwith("force_get_zopseq: expected ExpLineZ")
    | ExpLineZ(zopseq) => zopseq;
};

module ZBlock = {
  let wrap' = (zopseq: zopseq): zblock => ([], ExpLineZ(zopseq), []);
  let wrap = (zoperand: zoperand): zblock => wrap'(ZOpSeq.wrap(zoperand));
};

let rec is_before = (ze: t): bool => ze |> is_before_zblock
and is_before_zblock = ((prefix, zline, _): zblock): bool =>
  switch (prefix) {
  | []
  | [EmptyLine] => is_before_zline(zline)
  | _ => false
  }
and is_before_zline = (zline: zline): bool =>
  switch (zline) {
  | CursorL(cursor, CommentLine(_)) => cursor == OnDelim(0, Before)
  | CursorL(cursor, EmptyLine) => cursor == OnText(0)
  | CursorL(cursor, LetLine(_))
  | CursorL(cursor, LivelitDefLine(_)) => cursor == OnDelim(0, Before)
  | CursorL(_, ExpLine(_)) => false /* ghost node */
  | ExpLineZ(zopseq) => is_before_zopseq(zopseq)
  | LetLineZP(_)
  | LetLineZE(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZCaptures(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_)
  | LivelitDefLineZInit(_)
  | LivelitDefLineZUpdate(_)
  | LivelitDefLineZView(_)
  | LivelitDefLineZShape(_)
  | LivelitDefLineZExpand(_) => false
  }
and is_before_zopseq = zopseq => ZOpSeq.is_before(~is_before_zoperand, zopseq)
and is_before_zoperand =
  fun
  | CursorE(cursor, EmptyHole(_))
  | CursorE(cursor, ListNil(_)) => cursor == OnDelim(0, Before)
  | CursorE(cursor, InvalidText(_, _))
  | CursorE(cursor, Var(_))
  | CursorE(cursor, IntLit(_))
  | CursorE(cursor, FloatLit(_))
  | CursorE(cursor, BoolLit(_)) => cursor == OnText(0)
  | CursorE(cursor, StringLit(_))
  | CursorE(cursor, Lam(_))
  | CursorE(cursor, Inj(_))
  | CursorE(cursor, Case(_))
  | CursorE(cursor, Parenthesized(_)) => cursor == OnDelim(0, Before)
  | CursorE(cursor, ApLivelit(_) | FreeLivelit(_, _)) => cursor == OnText(0)
  | CursorE(_, Subscript(_)) => false
  | SubscriptZE1(_, e, _, _) => is_before(e)
  | ParenthesizedZ(_)
  | LamZP(_)
  | LamZE(_)
  | InjZ(_)
  | CaseZE(_)
  | CaseZR(_)
  | SubscriptZE2(_)
  | SubscriptZE3(_)
  | ApLivelitZ(_) => false;

// The following 2 functions are specifically for CommentLines!!
// Check if the cursor at "OnDelim(After)" in a "CommentLine"
/* For example:
           # Comment1
           #| Comment2

   */
let is_begin_of_comment = ((prefix, zline, _): zblock): bool =>
  switch (zline) {
  | CursorL(cursor, CommentLine(_)) =>
    switch (prefix |> ListUtil.split_last_opt) {
    | Some((_, CommentLine(_))) => cursor == OnDelim(0, After)
    | _ => false
    }
  | _ => false
  };
// Check if the cursor at the end of a "CommentLine"
/* For example:
           # Comment1|
           # Comment2

   */
let is_end_of_comment = ((_, zline, suffix): zblock): bool =>
  switch (zline) {
  | CursorL(cursor, CommentLine(comment)) =>
    switch (suffix) {
    | [CommentLine(_), ..._] => cursor == OnText(String.length(comment))
    | _ => false
    }
  | _ => false
  };

let is_before_zrule =
  fun
  | CursorR(OnDelim(0, Before), _) => true
  | _ => false;
let is_before_zoperator: zoperator => bool =
  fun
  | (OnOp(Before), _) => true
  | _ => false;

let rec is_after = (ze: t): bool => ze |> is_after_zblock
and is_after_zblock = ((_, zline, suffix): zblock): bool =>
  switch (suffix) {
  | [] => is_after_zline(zline)
  | _ => false
  }
and is_after_zline =
  fun
  | CursorL(cursor, CommentLine(comment)) =>
    cursor == OnText(String.length(comment))
  | CursorL(cursor, EmptyLine) => cursor == OnText(0)
  | CursorL(cursor, LetLine(_)) => cursor == OnDelim(3, After)
  | CursorL(cursor, LivelitDefLine(_)) => cursor == OnDelim(2, After)
  | CursorL(_, ExpLine(_)) => false /* ghost node */
  | ExpLineZ(zopseq) => is_after_zopseq(zopseq)
  | LetLineZP(_)
  | LetLineZE(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZCaptures(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_)
  | LivelitDefLineZInit(_)
  | LivelitDefLineZUpdate(_)
  | LivelitDefLineZView(_)
  | LivelitDefLineZShape(_)
  | LivelitDefLineZExpand(_) => false
and is_after_zopseq = zopseq => ZOpSeq.is_after(~is_after_zoperand, zopseq)
and is_after_zoperand =
  fun
  | CursorE(cursor, EmptyHole(_))
  | CursorE(cursor, ListNil(_)) => cursor == OnDelim(0, After)
  | CursorE(cursor, InvalidText(_, t)) =>
    cursor == OnText(String.length(t))
  | CursorE(cursor, Var(_, _, x)) => cursor == OnText(Var.length(x))
  | CursorE(cursor, IntLit(_, n)) => cursor == OnText(String.length(n))
  | CursorE(cursor, FloatLit(_, f)) => cursor == OnText(String.length(f))
  | CursorE(cursor, BoolLit(_, true)) => cursor == OnText(4)
  | CursorE(cursor, BoolLit(_, false)) => cursor == OnText(5)
  | CursorE(cursor, Lam(_)) => cursor == OnDelim(2, After)
  | CursorE(cursor, Case(_)) => cursor == OnDelim(1, After)
  | CursorE(cursor, Inj(_)) => cursor == OnDelim(1, After)
  | CursorE(cursor, Parenthesized(_)) => cursor == OnDelim(1, After)
  | CursorE(cursor, StringLit(_)) => cursor == OnDelim(1, After)
  | CursorE(cursor, Subscript(_)) => cursor == OnDelim(2, After)
  | CursorE(cursor, ApLivelit(_, _, _, name, _, _) | FreeLivelit(_, name)) =>
    cursor == OnText(LivelitName.length(name))
  | ParenthesizedZ(_) => false
  | LamZP(_)
  | LamZE(_)
  | InjZ(_)
  | CaseZE(_)
  | CaseZR(_)
  | SubscriptZE1(_)
  | SubscriptZE2(_)
  | SubscriptZE3(_)
  | ApLivelitZ(_) => false;

let is_after_zrule =
  fun
  | RuleZE(_, zclause) => is_after(zclause)
  | _ => false;
let is_after_zoperator: zoperator => bool =
  fun
  | (OnOp(After), _) => true
  | _ => false;

let rec is_outer = (ze: t): bool => ze |> is_outer_zblock
and is_outer_zblock = ((_, zline, suffix): zblock): bool =>
  switch (suffix) {
  | [] => is_outer_zline(zline)
  | _ => false
  }
and is_outer_zline = (zline: zline): bool =>
  switch (zline) {
  | CursorL(_, EmptyLine)
  | CursorL(_, CommentLine(_))
  | CursorL(_, LetLine(_))
  | CursorL(_, LivelitDefLine(_)) => true
  | CursorL(_, ExpLine(_)) => false /* ghost node */
  | ExpLineZ(zopseq) => is_outer_zopseq(zopseq)
  | LetLineZP(_)
  | LetLineZE(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZCaptures(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_)
  | LivelitDefLineZInit(_)
  | LivelitDefLineZUpdate(_)
  | LivelitDefLineZView(_)
  | LivelitDefLineZShape(_)
  | LivelitDefLineZExpand(_) => false
  }
and is_outer_zopseq = zopseq => ZOpSeq.is_outer(~is_outer_zoperand, zopseq)
and is_outer_zoperand =
  fun
  | CursorE(_, EmptyHole(_))
  | CursorE(_, InvalidText(_, _))
  | CursorE(_, ListNil(_))
  | CursorE(_, Var(_))
  | CursorE(_, IntLit(_))
  | CursorE(_, FloatLit(_))
  | CursorE(_, BoolLit(_))
  | CursorE(_, StringLit(_))
  | CursorE(_, Lam(_))
  | CursorE(_, Inj(_))
  | CursorE(_, Case(_))
  | CursorE(_, Parenthesized(_))
  | CursorE(_, Subscript(_))
  | CursorE(_, FreeLivelit(_))
  | CursorE(_, ApLivelit(_)) => true
  | ParenthesizedZ(zexp) => is_outer(zexp)
  | LamZP(_)
  | LamZE(_)
  | InjZ(_)
  | CaseZE(_)
  | CaseZR(_)
  | SubscriptZE1(_)
  | SubscriptZE2(_)
  | SubscriptZE3(_)
  | ApLivelitZ(_) => false;

let rec place_before = (e: UHExp.t): t => e |> place_before_block
and place_before_block =
  fun
  | [] => failwith("place_before_block: empty block")
  | [EmptyLine, first, ...rest] => (
      [EmptyLine],
      first |> place_before_line,
      rest,
    )
  | [first, ...rest] => ([], first |> place_before_line, rest)
and place_before_line =
  fun
  | CommentLine(_) as line => CursorL(OnDelim(0, Before), line)
  | EmptyLine => CursorL(OnText(0), EmptyLine)
  | (LetLine(_) | LivelitDefLine(_)) as line =>
    CursorL(OnDelim(0, Before), line)
  | ExpLine(opseq) => ExpLineZ(place_before_opseq(opseq))
and place_before_opseq = opseq =>
  ZOpSeq.place_before(~place_before_operand, opseq)
and place_before_operand = operand =>
  switch (operand) {
  | EmptyHole(_)
  | ListNil(_) => CursorE(OnDelim(0, Before), operand)
  | InvalidText(_, _)
  | Var(_)
  | IntLit(_)
  | FloatLit(_)
  | BoolLit(_) => CursorE(OnText(0), operand)
  | StringLit(_)
  | Lam(_)
  | Inj(_)
  | Case(_)
  | Parenthesized(_) => CursorE(OnDelim(0, Before), operand)
  | Subscript(err, target, start_, end_) =>
    SubscriptZE1(err, place_before(target), start_, end_)
  | ApLivelit(_)
  | FreeLivelit(_, _) => CursorE(OnText(0), operand)
  };
let place_before_rule = (rule: UHExp.rule): zrule =>
  CursorR(OnDelim(0, Before), rule);
let place_before_operator = (op: UHExp.operator): option(zoperator) =>
  switch (op) {
  | Space => None
  | _ => Some((OnOp(Before), op))
  };

let rec place_after = (e: UHExp.t): t => e |> place_after_block
and place_after_block = (block: UHExp.block): zblock =>
  switch (block |> ListUtil.split_last_opt) {
  | None => failwith("place_after_block: empty block")
  | Some((leading, last)) => (leading, last |> place_after_line, [])
  }
and place_after_line =
  fun
  | CommentLine(comment) as line =>
    CursorL(OnText(String.length(comment)), line)
  | EmptyLine => CursorL(OnText(0), EmptyLine)
  | LivelitDefLine(_) as line => CursorL(OnDelim(2, After), line)
  | LetLine(_) as line => CursorL(OnDelim(2, After), line)
  | ExpLine(e) => ExpLineZ(place_after_opseq(e))
and place_after_opseq = opseq =>
  ZOpSeq.place_after(~place_after_operand, opseq)
and place_after_operand = operand =>
  switch (operand) {
  | EmptyHole(_)
  | ListNil(_) => CursorE(OnDelim(0, After), operand)
  | InvalidText(_, t) => CursorE(OnText(String.length(t)), operand)
  | Var(_, _, x) => CursorE(OnText(Var.length(x)), operand)
  | IntLit(_, n) => CursorE(OnText(String.length(n)), operand)
  | FloatLit(_, f) => CursorE(OnText(String.length(f)), operand)
  | BoolLit(_, true) => CursorE(OnText(4), operand)
  | BoolLit(_, false) => CursorE(OnText(5), operand)
  | StringLit(_) => CursorE(OnDelim(1, After), operand)
  | Lam(_) => CursorE(OnDelim(2, After), operand)
  | Case(_) => CursorE(OnDelim(1, After), operand)
  | Inj(_) => CursorE(OnDelim(1, After), operand)
  | Parenthesized(_) => CursorE(OnDelim(1, After), operand)
  | Subscript(_) => CursorE(OnDelim(2, After), operand)
  | ApLivelit(_, _, _, llname, _, _)
  | FreeLivelit(_, llname) =>
    CursorE(OnText(LivelitName.length(llname)), operand)
  };
let place_after_rule = (Rule(p, clause): UHExp.rule): zrule =>
  RuleZE(p, place_after(clause));
let place_after_operator = (op: UHExp.operator): option(zoperator) =>
  switch (op) {
  | Space => None
  | _ => Some((OnOp(After), op))
  };

let place_cursor_operator =
    (cursor: CursorPosition.t, operator: UHExp.operator): option(zoperator) =>
  is_valid_cursor_operator(cursor, operator)
    ? Some((cursor, operator)) : None;
let place_cursor_operand =
    (cursor: CursorPosition.t, operand: UHExp.operand): option(zoperand) =>
  is_valid_cursor_operand(cursor, operand)
    ? Some(CursorE(cursor, operand)) : None;
let place_cursor_line =
    (cursor: CursorPosition.t, line: UHExp.line): option(zline) =>
  switch (line) {
  | ExpLine(_) =>
    // all cursor positions in a zopseq are
    // encoded in steps, not CursorPosition.t
    None
  | EmptyLine
  | CommentLine(_)
  | LetLine(_)
  | LivelitDefLine(_) =>
    is_valid_cursor_line(cursor, line) ? Some(CursorL(cursor, line)) : None
  };
let place_cursor_rule =
    (cursor: CursorPosition.t, rule: UHExp.rule): option(zrule) =>
  is_valid_cursor_rule(cursor, rule) ? Some(CursorR(cursor, rule)) : None;

let prune_empty_hole_line = (zli: zline): zline =>
  switch (zli) {
  | ExpLineZ(ZOpSeq(_, ZOperand(CursorE(_, EmptyHole(_)), (E, E)))) =>
    place_before_line(EmptyLine)
  | ExpLineZ(_)
  | LetLineZP(_)
  | LetLineZE(_)
  | CursorL(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZCaptures(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_)
  | LivelitDefLineZInit(_)
  | LivelitDefLineZUpdate(_)
  | LivelitDefLineZView(_)
  | LivelitDefLineZShape(_)
  | LivelitDefLineZExpand(_) => zli
  };
let prune_empty_hole_lines = ((prefix, zline, suffix): zblock): zblock =>
  switch (suffix) {
  | [] => (prefix |> UHExp.Lines.prune_empty_holes, zline, [])
  | [_, ..._] => (
      prefix |> UHExp.Lines.prune_empty_holes,
      prune_empty_hole_line(zline),
      suffix |> UHExp.Block.prune_empty_hole_lines,
    )
  };

let rec erase = (ze: t): UHExp.t => ze |> erase_zblock
and erase_zblock = ((prefix, zline, suffix): zblock): UHExp.block =>
  prefix @ [zline |> erase_zline] @ suffix
and erase_zline =
  fun
  | CursorL(_, line) => line
  | ExpLineZ(zopseq) => ExpLine(erase_zopseq(zopseq))
  | LetLineZP(err, zp, def) => LetLine(err, ZPat.erase(zp), def)
  | LetLineZE(err, p, zdef) => LetLine(err, p, erase(zdef))
  | LivelitDefLineZExpansionType({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type: ZTyp.erase(expansion_type),
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    })
  | LivelitDefLineZCaptures({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures: erase(captures),
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    })
  | LivelitDefLineZModelType({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type: ZTyp.erase(model_type),
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    })
  | LivelitDefLineZActionType({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type: ZTyp.erase(action_type),
      init,
      update,
      view,
      shape,
      expand,
    })
  | LivelitDefLineZInit({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init: erase(init),
      update,
      view,
      shape,
      expand,
    })
  | LivelitDefLineZUpdate({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update: erase(update),
      view,
      shape,
      expand,
    })
  | LivelitDefLineZView({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view: erase(view),
      shape,
      expand,
    })
  | LivelitDefLineZShape({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape: erase(shape),
      expand,
    })
  | LivelitDefLineZExpand({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand,
    }) =>
    LivelitDefLine({
      name,
      captures,
      expansion_type,
      model_type,
      action_type,
      init,
      update,
      view,
      shape,
      expand: erase(expand),
    })
and erase_zopseq = zopseq =>
  ZOpSeq.erase(~erase_zoperand, ~erase_zoperator, zopseq)
and erase_zoperator =
  fun
  | (_, operator) => operator
and erase_zoperand =
  fun
  | CursorE(_, operand) => operand
  | ParenthesizedZ(zbody) => Parenthesized(erase(zbody))
  | LamZP(err, zp, body) => Lam(err, ZPat.erase(zp), body)
  | LamZE(err, p, zbody) => Lam(err, p, erase(zbody))
  | InjZ(err, side, zbody) => Inj(err, side, erase(zbody))
  | CaseZE(err, zscrut, rules) => Case(err, erase(zscrut), rules)
  | CaseZR(err, scrut, zrules) => Case(err, scrut, erase_zrules(zrules))
  | SubscriptZE1(err, ztarget, start_, end_) =>
    Subscript(err, erase(ztarget), start_, end_)
  | SubscriptZE2(err, target, zstart_, end_) =>
    Subscript(err, target, erase(zstart_), end_)
  | SubscriptZE3(err, target, start_, zend_) =>
    Subscript(err, target, start_, erase(zend_))
  | ApLivelitZ(llu, err, base_name, palette_name, serialized_model, zpsi) => {
      let psi = ZSpliceInfo.erase(zpsi, ((ty, z)) => (ty, erase(z)));
      ApLivelit(llu, err, base_name, palette_name, serialized_model, psi);
    }
and erase_zrules =
  fun
  | zrules => ZList.erase(zrules, erase_zrule)
and erase_zrule =
  fun
  | CursorR(_, rule) => rule
  | RuleZP(zp, clause) => Rule(ZPat.erase(zp), clause)
  | RuleZE(p, zclause) => Rule(p, erase(zclause));

let erase_zseq = ZSeq.erase(~erase_zoperand, ~erase_zoperator);

let mk_ZOpSeq =
  ZOpSeq.mk(~associate=UHExp.associate, ~erase_zoperand, ~erase_zoperator);

let get_err_status = ze => ze |> erase |> UHExp.get_err_status;
let get_err_status_zblock = zblock =>
  zblock |> erase_zblock |> UHExp.get_err_status_block;

let get_err_status_zopseq = zopseq =>
  zopseq |> erase_zopseq |> UHExp.get_err_status_opseq;
let get_err_status_zoperand = zoperand =>
  zoperand |> erase_zoperand |> UHExp.get_err_status_operand;

let rec set_err_status = (err: ErrStatus.t, ze: t): t =>
  ze |> set_err_status_zblock(err)
and set_err_status_zblock =
    (err: ErrStatus.t, (prefix, zline, suffix): zblock): zblock =>
  switch (suffix |> ListUtil.split_last_opt) {
  | None =>
    let zopseq = zline |> ZLine.force_get_zopseq;
    (prefix, ExpLineZ(zopseq |> set_err_status_zopseq(err)), []);
  | Some((suffix_leading, suffix_last)) =>
    let opseq = suffix_last |> UHExp.Line.force_get_opseq;
    (
      prefix,
      zline,
      suffix_leading @ [ExpLine(opseq |> UHExp.set_err_status_opseq(err))],
    );
  }
and set_err_status_zopseq = (err, zopseq) =>
  ZOpSeq.set_err_status(~set_err_status_zoperand, err, zopseq)
and set_err_status_zoperand = (err, zoperand) =>
  switch (zoperand) {
  | CursorE(cursor, operand) =>
    CursorE(cursor, UHExp.set_err_status_operand(err, operand))
  | ParenthesizedZ(zbody) => ParenthesizedZ(set_err_status(err, zbody))
  | LamZP(_, zp, body) => LamZP(err, zp, body)
  | LamZE(_, p, zbody) => LamZE(err, p, zbody)
  | InjZ(_, inj_side, zbody) => InjZ(err, inj_side, zbody)
  | CaseZE(_, zscrut, rules) =>
    CaseZE(StandardErrStatus(err), zscrut, rules)
  | CaseZR(_, scrut, zrules) =>
    CaseZR(StandardErrStatus(err), scrut, zrules)
  | SubscriptZE1(_, ztarget, start_, end_) =>
    SubscriptZE1(err, ztarget, start_, end_)
  | SubscriptZE2(_, target, zstart_, end_) =>
    SubscriptZE2(err, target, zstart_, end_)
  | SubscriptZE3(_, target, start_, zend_) =>
    SubscriptZE3(err, target, start_, zend_)
  | ApLivelitZ(llu, _, base_name, name, model, psi) =>
    ApLivelitZ(llu, err, base_name, name, model, psi)
  };

let rec mk_inconsistent = (u_gen: MetaVarGen.t, ze: t): (t, MetaVarGen.t) =>
  ze |> mk_inconsistent_zblock(u_gen)
and mk_inconsistent_zblock =
    (u_gen: MetaVarGen.t, (prefix, zline, suffix): zblock)
    : (zblock, MetaVarGen.t) =>
  switch (suffix |> ListUtil.split_last_opt) {
  | None =>
    let (zconclusion, u_gen) =
      zline |> ZLine.force_get_zopseq |> mk_inconsistent_zopseq(u_gen);
    ((prefix, ExpLineZ(zconclusion), []), u_gen);
  | Some((suffix_leading, suffix_last)) =>
    let (conclusion, u_gen) =
      suffix_last
      |> UHExp.Line.force_get_opseq
      |> UHExp.mk_inconsistent_opseq(u_gen);
    ((prefix, zline, suffix_leading @ [ExpLine(conclusion)]), u_gen);
  }
and mk_inconsistent_zopseq = (u_gen, zopseq) =>
  ZOpSeq.mk_inconsistent(~mk_inconsistent_zoperand, u_gen, zopseq)
and mk_inconsistent_zoperand = (u_gen, zoperand) =>
  switch (zoperand) {
  | CursorE(cursor, operand) =>
    let (operand, u_gen) = operand |> UHExp.mk_inconsistent_operand(u_gen);
    (CursorE(cursor, operand), u_gen);
  | ParenthesizedZ(zbody) =>
    let (zbody, u_gen) = mk_inconsistent(u_gen, zbody);
    (ParenthesizedZ(zbody), u_gen);
  /* already in hole */
  | LamZP(InHole(TypeInconsistent(_), _), _, _)
  | LamZE(InHole(TypeInconsistent(_), _), _, _)
  | InjZ(InHole(TypeInconsistent(_), _), _, _)
  | CaseZE(StandardErrStatus(InHole(TypeInconsistent(_), _)), _, _)
  | CaseZR(StandardErrStatus(InHole(TypeInconsistent(_), _)), _, _)
  | SubscriptZE1(InHole(TypeInconsistent(_), _), _, _, _)
  | SubscriptZE2(InHole(TypeInconsistent(_), _), _, _, _)
  | SubscriptZE3(InHole(TypeInconsistent(_), _), _, _, _)
  | ApLivelitZ(_, InHole(TypeInconsistent(_), _), _, _, _, _) => (
      zoperand,
      u_gen,
    )
  /* not in hole */
  | LamZP(NotInHole | InHole(WrongLength, _), _, _)
  | LamZE(NotInHole | InHole(WrongLength, _), _, _)
  | InjZ(NotInHole | InHole(WrongLength, _), _, _)
  | CaseZE(
      StandardErrStatus(NotInHole | InHole(WrongLength, _)) |
      InconsistentBranches(_, _),
      _,
      _,
    )
  | CaseZR(
      StandardErrStatus(NotInHole | InHole(WrongLength, _)) |
      InconsistentBranches(_, _),
      _,
      _,
    )
  | SubscriptZE1(NotInHole | InHole(WrongLength, _), _, _, _)
  | SubscriptZE2(NotInHole | InHole(WrongLength, _), _, _, _)
  | SubscriptZE3(NotInHole | InHole(WrongLength, _), _, _, _)
  | ApLivelitZ(_, NotInHole | InHole(WrongLength, _), _, _, _, _) =>
    let (u, u_gen) = u_gen |> MetaVarGen.next_hole;
    let zoperand =
      zoperand |> set_err_status_zoperand(InHole(TypeInconsistent(None), u));
    (zoperand, u_gen);
  };
let new_EmptyHole = (u_gen: MetaVarGen.t): (zoperand, MetaVarGen.t) => {
  let (hole, u_gen) = UHExp.new_EmptyHole(u_gen);
  (place_before_operand(hole), u_gen);
};

let empty_zrule = (u_gen: MetaVarGen.t): (zrule, MetaVarGen.t) => {
  let (zp, u_gen) = ZPat.new_EmptyHole(u_gen);
  let (clause, u_gen) = UHExp.new_EmptyHole(u_gen);
  let zrule = RuleZP(ZOpSeq.wrap(zp), UHExp.Block.wrap(clause));
  (zrule, u_gen);
};

let is_inconsistent = zoperand =>
  zoperand |> erase_zoperand |> UHExp.is_inconsistent;

let rec move_cursor_left = (ze: t): option(t) =>
  ze |> move_cursor_left_zblock
and move_cursor_left_zblock =
  fun
  | (prefix, zline, suffix) =>
    switch (move_cursor_left_zline(zline)) {
    | Some(zline) => Some((prefix, zline, suffix))
    | None =>
      switch (prefix |> ListUtil.split_last_opt) {
      | None
      | Some(([], EmptyLine)) => None
      | Some((prefix_leading, prefix_last)) =>
        Some((
          prefix_leading,
          prefix_last |> place_after_line,
          [zline |> erase_zline, ...suffix],
        ))
      }
    }
and move_cursor_left_zline = (zline: zline): option(zline) =>
  switch (zline) {
  | _ when is_before_zline(zline) => None

  | CursorL(OnOp(_), _) => None
  | CursorL(OnDelim(k, After), line) =>
    Some(CursorL(OnDelim(k, Before), line))
  | CursorL(OnDelim(_), EmptyLine | CommentLine(_) | ExpLine(_)) => None
  | CursorL(OnText(0), LivelitDefLine(llrecord)) =>
    Some(CursorL(OnDelim(0, After), LivelitDefLine(llrecord)))
  | CursorL(OnText(n), LivelitDefLine(llrecord)) =>
    Some(CursorL(OnText(n - 1), LivelitDefLine(llrecord)))

  | CursorL(OnText(_), EmptyLine) => None
  | CursorL(OnText(0), CommentLine(_) as line) =>
    Some(CursorL(OnDelim(0, After), line))
  | CursorL(OnText(k), CommentLine(_) as line) =>
    Some(CursorL(OnText(k - 1), line))
  | CursorL(OnText(_), ExpLine(_) | LetLine(_)) => None

  | CursorL(OnDelim(k, Before), LetLine(err, p, def)) =>
    switch (k) {
    | 1 => Some(LetLineZP(err, ZPat.place_after(p), def))
    | _ => Some(LetLineZE(err, p, place_after(def)))
    }

  | CursorL(
      OnDelim(k, Before),
      LivelitDefLine(
        {
          name: (_err, name_str) as name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        } as llrecord,
      ),
    ) =>
    switch (k) {
    | 1 /* "at" */ =>
      Some(
        CursorL(
          OnText(String.length(name_str) - 1),
          LivelitDefLine(llrecord),
        ),
      )
    | 2 /* "{" */ =>
      Some(
        LivelitDefLineZExpansionType({
          name,
          expansion_type: ZTyp.place_after(expansion_type),
          captures,
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    | 3 /* "}" */ =>
      Some(
        LivelitDefLineZExpand({
          name,
          expansion_type,
          captures,
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand: place_after(expand),
        }),
      )
    | _ => None // invalid cursor position
    }

  | ExpLineZ(zopseq) =>
    switch (move_cursor_left_zopseq(zopseq)) {
    | None => None
    | Some(zopseq) => Some(ExpLineZ(zopseq))
    }

  | LivelitDefLineZExpansionType({expansion_type, _} as llrecord) =>
    switch (ZTyp.move_cursor_left(expansion_type)) {
    | Some(expansion_type) =>
      Some(LivelitDefLineZExpansionType({...llrecord, expansion_type}))
    | None => Some(CursorL(OnDelim(1, After), erase_zline(zline)))
    }
  | LivelitDefLineZCaptures({captures, _} as llrecord) =>
    switch (move_cursor_left(captures)) {
    | Some(captures) =>
      Some(LivelitDefLineZCaptures({...llrecord, captures}))
    | None => Some(CursorL(OnDelim(2, After), erase_zline(zline)))
    }
  | LivelitDefLineZModelType(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (ZTyp.move_cursor_left(model_type)) {
    | Some(model_type) =>
      Some(LivelitDefLineZModelType({...llrecord, model_type}))
    | None =>
      Some(
        LivelitDefLineZCaptures({
          name,
          expansion_type,
          captures: place_after(captures),
          model_type: ZTyp.erase(model_type),
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZActionType(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (ZTyp.move_cursor_left(action_type)) {
    | Some(action_type) =>
      Some(LivelitDefLineZActionType({...llrecord, action_type}))
    | None =>
      Some(
        LivelitDefLineZModelType({
          name,
          expansion_type,
          captures,
          model_type: ZTyp.place_after(model_type),
          action_type: ZTyp.erase(action_type),
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZInit(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_left(init)) {
    | Some(init) => Some(LivelitDefLineZInit({...llrecord, init}))
    | None =>
      Some(
        LivelitDefLineZActionType({
          name,
          expansion_type,
          captures,
          model_type,
          action_type: ZTyp.place_after(action_type),
          init: erase(init),
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZUpdate(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_left(update)) {
    | Some(update) => Some(LivelitDefLineZUpdate({...llrecord, update}))
    | None =>
      Some(
        LivelitDefLineZInit({
          name,
          expansion_type,
          captures,
          model_type,
          action_type,
          init: place_after(init),
          update: erase(update),
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZView(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_left(view)) {
    | Some(view) => Some(LivelitDefLineZView({...llrecord, view}))
    | None =>
      Some(
        LivelitDefLineZUpdate({
          name,
          expansion_type,
          captures,
          model_type,
          action_type,
          init,
          update: place_after(update),
          view: erase(view),
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZShape(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_left(shape)) {
    | Some(shape) => Some(LivelitDefLineZShape({...llrecord, shape}))
    | None =>
      Some(
        LivelitDefLineZView({
          name,
          expansion_type,
          captures,

          model_type,
          action_type,
          init,
          update,
          view: place_after(view),
          shape: erase(shape),
          expand,
        }),
      )
    }
  | LivelitDefLineZExpand(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_left(expand)) {
    | Some(expand) => Some(LivelitDefLineZExpand({...llrecord, expand}))
    | None =>
      Some(
        LivelitDefLineZShape({
          name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init,
          update,
          view,
          shape: place_after(shape),
          expand: erase(expand),
        }),
      )
    }
  | LetLineZP(err, zp, def) =>
    switch (ZPat.move_cursor_left(zp)) {
    | Some(zp) => Some(LetLineZP(err, zp, def))
    | None =>
      Some(CursorL(OnDelim(0, After), LetLine(err, ZPat.erase(zp), def)))
    }
  | LetLineZE(err, p, zdef) =>
    switch (move_cursor_left(zdef)) {
    | Some(zdef) => Some(LetLineZE(err, p, zdef))
    | None =>
      Some(CursorL(OnDelim(1, After), LetLine(err, p, erase(zdef))))
    }
  }
and move_cursor_left_zopseq = zopseq =>
  ZOpSeq.move_cursor_left(
    ~move_cursor_left_zoperand,
    ~move_cursor_left_zoperator,
    ~place_after_operand,
    ~place_after_operator,
    ~erase_zoperand,
    ~erase_zoperator,
    zopseq,
  )
and move_cursor_left_zoperator =
  fun
  | (OnText(_) | OnDelim(_, _), _) => None
  | (OnOp(Before), _) => None
  | (OnOp(After), op) => Some((OnOp(Before), op))
and move_cursor_left_zoperand =
  fun
  | z when is_before_zoperand(z) => None
  | CursorE(OnOp(_), _) => None
  | CursorE(OnText(j), e) => Some(CursorE(OnText(j - 1), e))
  | CursorE(OnDelim(k, After), e) => Some(CursorE(OnDelim(k, Before), e))
  | CursorE(OnDelim(_, Before), EmptyHole(_) | ListNil(_)) => None
  | CursorE(OnDelim(_one, Before), StringLit(_, s) as operand) =>
    Some(CursorE(OnText(String.length(s)), operand))
  | CursorE(OnDelim(_one, Before), Parenthesized(body)) =>
    Some(ParenthesizedZ(place_after(body)))
  | CursorE(OnDelim(_one, Before), Inj(err, side, body)) =>
    Some(InjZ(err, side, place_after(body)))
  | CursorE(OnDelim(k, Before), Lam(err, arg, body)) =>
    switch (k) {
    | 1 => Some(LamZP(err, ZPat.place_after(arg), body))
    | 2 => Some(LamZE(err, arg, place_after(body)))
    | _ => None
    }
  | CursorE(OnDelim(_k, Before), Case(err, scrut, rules)) =>
    // _k == 1
    switch (List.rev(rules)) {
    | [] => Some(CaseZE(err, place_after(scrut), rules))
    | [last_rule, ...rev_prefix] =>
      Some(
        CaseZR(
          err,
          scrut,
          (List.rev(rev_prefix), place_after_rule(last_rule), []),
        ),
      )
    }
  | CursorE(OnDelim(k, Before), Subscript(err, target, start_, end_)) =>
    if (k == 0) {
      Some(SubscriptZE1(err, place_after(target), start_, end_));
    } else if (k == 1) {
      Some(SubscriptZE2(err, target, place_after(start_), end_));
    } else {
      Some(SubscriptZE3(err, target, start_, place_after(end_)));
    }
  | CursorE(
      OnDelim(_),
      InvalidText(_) | Var(_) | BoolLit(_) | IntLit(_) | FloatLit(_) |
      ApLivelit(_) |
      FreeLivelit(_),
    ) =>
    // invalid cursor position
    None
  | ParenthesizedZ(zbody) =>
    switch (move_cursor_left(zbody)) {
    | Some(zbody) => Some(ParenthesizedZ(zbody))
    | None =>
      Some(CursorE(OnDelim(0, After), Parenthesized(erase(zbody))))
    }
  | InjZ(err, side, zbody) =>
    switch (move_cursor_left(zbody)) {
    | Some(zbody) => Some(InjZ(err, side, zbody))
    | None =>
      Some(CursorE(OnDelim(0, After), Inj(err, side, erase(zbody))))
    }
  | LamZP(err, zarg, body) =>
    switch (ZPat.move_cursor_left(zarg)) {
    | Some(zarg) => Some(LamZP(err, zarg, body))
    | None =>
      Some(CursorE(OnDelim(0, After), Lam(err, ZPat.erase(zarg), body)))
    }
  | LamZE(err, arg, zbody) =>
    switch (move_cursor_left(zbody)) {
    | Some(zbody) => Some(LamZE(err, arg, zbody))
    | None =>
      Some(CursorE(OnDelim(1, After), Lam(err, arg, erase(zbody))))
    }
  | CaseZE(err, zscrut, rules) =>
    switch (move_cursor_left(zscrut)) {
    | Some(zscrut) => Some(CaseZE(err, zscrut, rules))
    | None =>
      Some(CursorE(OnDelim(0, After), Case(err, erase(zscrut), rules)))
    }
  | CaseZR(err, scrut, zrules) =>
    switch (zrules |> move_cursor_left_zrules) {
    | Some(zrules) => Some(CaseZR(err, scrut, zrules))
    | None => Some(CaseZE(err, scrut |> place_after, zrules |> erase_zrules))
    }
  | SubscriptZE1(err, ztarget, start_, end_) =>
    switch (move_cursor_left(ztarget)) {
    | Some(ztarget) => Some(SubscriptZE1(err, ztarget, start_, end_))
    | None => None
    }
  | SubscriptZE2(err, target, zstart_, end_) =>
    switch (move_cursor_left(zstart_)) {
    | Some(zstart_) => Some(SubscriptZE2(err, target, zstart_, end_))
    | None =>
      Some(
        CursorE(
          OnDelim(0, After),
          Subscript(err, target, erase(zstart_), end_),
        ),
      )
    }
  | SubscriptZE3(err, target, start_, zend_) =>
    switch (move_cursor_left(zend_)) {
    | Some(zend_) => Some(SubscriptZE3(err, target, start_, zend_))
    | None =>
      Some(
        CursorE(
          OnDelim(1, After),
          Subscript(err, target, start_, erase(zend_)),
        ),
      )
    }
  | ApLivelitZ(llu, err, base_name, name, model, zsi) => {
      let (ty, ze) = ZSpliceInfo.prj_z(zsi);
      switch (move_cursor_left(ze)) {
      | Some(ze) =>
        let zsi = ZSpliceInfo.update_z(zsi, (ty, ze));
        Some(ApLivelitZ(llu, err, base_name, name, model, zsi));
      | None => None
      };
    }
and move_cursor_left_zrules =
  fun
  | (prefix, zrule, suffix) =>
    switch (move_cursor_left_zrule(zrule)) {
    | Some(zrule) => Some((prefix, zrule, suffix))
    | None =>
      switch (List.rev(prefix)) {
      | [] => None
      | [rule_before, ...rev_prefix] =>
        Some((
          List.rev(rev_prefix),
          place_after_rule(rule_before),
          [erase_zrule(zrule), ...suffix],
        ))
      }
    }
and move_cursor_left_zrule =
  fun
  | z when is_before_zrule(z) => None
  | CursorR(OnOp(_), _) => None
  | CursorR(OnText(_), _) => None
  | CursorR(OnDelim(k, After), rule) =>
    Some(CursorR(OnDelim(k, Before), rule))
  | CursorR(OnDelim(_one, Before), Rule(p, clause)) =>
    Some(RuleZP(ZPat.place_after(p), clause))
  | RuleZP(zp, clause) =>
    switch (ZPat.move_cursor_left(zp)) {
    | Some(zp) => Some(RuleZP(zp, clause))
    | None =>
      Some(CursorR(OnDelim(0, After), Rule(ZPat.erase(zp), clause)))
    }
  | RuleZE(p, zclause) =>
    switch (move_cursor_left(zclause)) {
    | Some(zclause) => Some(RuleZE(p, zclause))
    | None => Some(CursorR(OnDelim(1, After), Rule(p, erase(zclause))))
    };

let rec move_cursor_right = (ze: t): option(t) =>
  ze |> move_cursor_right_zblock
and move_cursor_right_zblock =
  fun
  | (prefix, zline, suffix) =>
    switch (move_cursor_right_zline(zline)) {
    | Some(zline) => Some((prefix, zline, suffix))
    | None =>
      switch (suffix) {
      | [] => None
      | [suffix_first, ...suffix_trailing] =>
        Some((
          prefix @ [erase_zline(zline)],
          place_before_line(suffix_first),
          suffix_trailing,
        ))
      }
    }
and move_cursor_right_zline =
  fun
  | z when is_after_zline(z) => None
  | CursorL(OnOp(_), _) => None
  | CursorL(OnText(k), CommentLine(_) as line) =>
    Some(CursorL(OnText(k + 1), line))
  | CursorL(OnText(_), EmptyLine | ExpLine(_) | LetLine(_)) => None
  | CursorL(OnDelim(k, Before), line) =>
    Some(CursorL(OnDelim(k, After), line))

  | CursorL(OnDelim(_, After), CommentLine(_) as line) =>
    Some(CursorL(OnText(0), line))

  | CursorL(OnDelim(_, _), EmptyLine | ExpLine(_)) => None
  | CursorL(OnDelim(k, After), LetLine(err, p, def)) =>
    switch (k) {
    | 0 => Some(LetLineZP(err, ZPat.place_before(p), def))
    | 1 => Some(LetLineZE(err, p, place_before(def)))
    | _ => None
    }
  | CursorL(
      OnDelim(k, After),
      LivelitDefLine(
        {
          name,
          expansion_type,
          captures,
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        } as llrecord,
      ),
    ) =>
    // k=0 on livelit. move to name ie text
    // k=1 on at. move to expansion_type
    // k=2 on {. move to captures
    switch (k) {
    | 0 => Some(CursorL(OnText(0), LivelitDefLine(llrecord)))
    | 1 =>
      Some(
        LivelitDefLineZExpansionType({
          name,
          expansion_type: ZTyp.place_before(expansion_type),
          captures,
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    | 2 =>
      Some(
        LivelitDefLineZCaptures({
          name,
          expansion_type,
          captures: place_before(captures),
          model_type,
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    | _ => None // invalid cursor position
    }
  | CursorL(
      OnText(n),
      LivelitDefLine({name: (_, name_str), _} as llrecord),
    ) =>
    if (n == 0) {
      None;
    } else if (n < String.length(name_str)) {
      Some(CursorL(OnText(n - 1), LivelitDefLine(llrecord)));
    } else {
      // move to "at"
      Some(
        CursorL(OnDelim(1, After), LivelitDefLine(llrecord)),
      );
    }
  | ExpLineZ(zopseq) =>
    switch (move_cursor_right_zopseq(zopseq)) {
    | None => None
    | Some(zopseq) => Some(ExpLineZ(zopseq))
    }
  | LivelitDefLineZExpansionType({expansion_type, _} as llrecord) =>
    switch (ZTyp.move_cursor_right(expansion_type)) {
    | Some(expansion_type) =>
      Some(LivelitDefLineZExpansionType({...llrecord, expansion_type}))
    | None =>
      // move cursor to "{"
      Some(
        CursorL(
          OnDelim(2, Before),
          erase_zline(LivelitDefLineZExpansionType(llrecord)),
        ),
      )
    }
  | LivelitDefLineZCaptures(
      {
        name,
        expansion_type,
        captures,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_right(captures)) {
    | Some(captures) =>
      Some(LivelitDefLineZCaptures({...llrecord, captures}))
    | None =>
      Some(
        LivelitDefLineZModelType({
          name,
          expansion_type,
          captures: erase(captures),
          model_type: ZTyp.place_before(model_type),
          action_type,
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZModelType(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (ZTyp.move_cursor_right(model_type)) {
    | Some(model_type) =>
      Some(LivelitDefLineZModelType({...llrecord, model_type}))
    | None =>
      Some(
        LivelitDefLineZActionType({
          name,
          captures,
          expansion_type,
          model_type: ZTyp.erase(model_type),
          action_type: ZTyp.place_before(action_type),
          init,
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZActionType(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (ZTyp.move_cursor_right(action_type)) {
    | Some(action_type) =>
      Some(LivelitDefLineZActionType({...llrecord, action_type}))
    | None =>
      Some(
        LivelitDefLineZInit({
          name,
          captures,
          expansion_type,
          model_type,
          action_type: ZTyp.erase(action_type),
          init: place_before(init),
          update,
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZInit(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_right(init)) {
    | Some(init) => Some(LivelitDefLineZInit({...llrecord, init}))
    | None =>
      Some(
        LivelitDefLineZUpdate({
          name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init: erase(init),
          update: place_before(update),
          view,
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZUpdate(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_right(update)) {
    | Some(update) => Some(LivelitDefLineZUpdate({...llrecord, update}))
    | None =>
      Some(
        LivelitDefLineZView({
          name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init,
          update: erase(update),
          view: place_before(view),
          shape,
          expand,
        }),
      )
    }
  | LivelitDefLineZView(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_right(view)) {
    | Some(view) => Some(LivelitDefLineZView({...llrecord, view}))
    | None =>
      Some(
        LivelitDefLineZShape({
          name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init,
          update,
          view: erase(view),
          shape: place_before(shape),
          expand,
        }),
      )
    }
  | LivelitDefLineZShape(
      {
        name,
        captures,
        expansion_type,
        model_type,
        action_type,
        init,
        update,
        view,
        shape,
        expand,
      } as llrecord,
    ) =>
    switch (move_cursor_right(shape)) {
    | Some(shape) => Some(LivelitDefLineZShape({...llrecord, shape}))
    | None =>
      Some(
        LivelitDefLineZExpand({
          name,
          captures,
          expansion_type,
          model_type,
          action_type,
          init,
          update,
          view,
          shape: erase(shape),
          expand: place_before(expand),
        }),
      )
    }
  | LivelitDefLineZExpand({expand, _} as llrecord) =>
    switch (move_cursor_right(expand)) {
    | Some(expand) => Some(LivelitDefLineZExpand({...llrecord, expand}))
    | None =>
      // move cursor to "}"
      Some(
        CursorL(
          OnDelim(3, Before),
          erase_zline(LivelitDefLineZExpand(llrecord)),
        ),
      )
    }
  | LetLineZP(err, zp, def) =>
    switch (ZPat.move_cursor_right(zp)) {
    | Some(zp) => Some(LetLineZP(err, zp, def))
    | None =>
      Some(CursorL(OnDelim(1, Before), LetLine(err, ZPat.erase(zp), def)))
    }
  | LetLineZE(err, p, zdef) =>
    switch (move_cursor_right(zdef)) {
    | Some(zdef) => Some(LetLineZE(err, p, zdef))
    | None =>
      Some(CursorL(OnDelim(2, Before), LetLine(err, p, erase(zdef))))
    }
and move_cursor_right_zopseq = zopseq =>
  ZOpSeq.move_cursor_right(
    ~move_cursor_right_zoperand,
    ~move_cursor_right_zoperator,
    ~place_before_operand,
    ~place_before_operator,
    ~erase_zoperand,
    ~erase_zoperator,
    zopseq,
  )
and move_cursor_right_zoperator =
  fun
  | (OnText(_) | OnDelim(_, _), _) => None
  | (OnOp(After), _) => None
  | (OnOp(Before), op) => Some((OnOp(After), op))
and move_cursor_right_zoperand =
  fun
  | z when is_after_zoperand(z) => None
  | CursorE(OnOp(_), _) => None
  | CursorE(OnText(j), e) => Some(CursorE(OnText(j + 1), e))
  | CursorE(OnDelim(k, Before), e) => Some(CursorE(OnDelim(k, After), e))
  | CursorE(OnDelim(_, After), EmptyHole(_) | ListNil(_)) => None
  | CursorE(OnDelim(_zero, After), StringLit(_) as operand) =>
    Some(CursorE(OnText(0), operand))
  | CursorE(OnDelim(_zero, After), Parenthesized(body)) =>
    Some(ParenthesizedZ(place_before(body)))
  | CursorE(OnDelim(_zero, After), Inj(err, side, body)) =>
    Some(InjZ(err, side, place_before(body)))
  | CursorE(OnDelim(k, After), Lam(err, arg, body)) =>
    switch (k) {
    | 0 => Some(LamZP(err, ZPat.place_before(arg), body))
    | 1 => Some(LamZE(err, arg, place_before(body)))
    | _ => None // invalid cursor position
    }
  | CursorE(OnDelim(_k, After), Case(err, scrut, rules)) =>
    // _k == 0
    Some(CaseZE(err, place_before(scrut), rules))
  | CursorE(OnDelim(k, After), Subscript(err, target, start_, end_)) =>
    k == 0
      ? Some(SubscriptZE2(err, target, place_before(start_), end_))
      : Some(SubscriptZE3(err, target, start_, place_before(end_)))
  | CursorE(
      OnDelim(_),
      InvalidText(_) | Var(_) | BoolLit(_) | IntLit(_) | FloatLit(_) |
      ApLivelit(_) |
      FreeLivelit(_),
    ) =>
    // invalid cursor position
    None
  | ParenthesizedZ(zbody) =>
    switch (move_cursor_right(zbody)) {
    | Some(zbody) => Some(ParenthesizedZ(zbody))
    | None =>
      Some(CursorE(OnDelim(1, Before), Parenthesized(erase(zbody))))
    }
  | InjZ(err, side, zbody) =>
    switch (move_cursor_right(zbody)) {
    | Some(zbody) => Some(InjZ(err, side, zbody))
    | None =>
      Some(CursorE(OnDelim(1, Before), Inj(err, side, erase(zbody))))
    }
  | LamZP(err, zarg, body) =>
    switch (ZPat.move_cursor_right(zarg)) {
    | Some(zarg) => Some(LamZP(err, zarg, body))
    | None =>
      Some(CursorE(OnDelim(1, Before), Lam(err, ZPat.erase(zarg), body)))
    }
  | LamZE(err, arg, zbody) =>
    switch (move_cursor_right(zbody)) {
    | None =>
      Some(CursorE(OnDelim(2, Before), Lam(err, arg, erase(zbody))))
    | Some(zbody) => Some(LamZE(err, arg, zbody))
    }
  | CaseZE(err, zscrut, rules) =>
    switch (move_cursor_right(zscrut)) {
    | Some(zscrut) => Some(CaseZE(err, zscrut, rules))
    | None =>
      switch (rules) {
      | [] =>
        Some(CursorE(OnDelim(1, Before), Case(err, erase(zscrut), rules)))
      | [r, ...rs] =>
        Some(CaseZR(err, erase(zscrut), ([], place_before_rule(r), rs)))
      }
    }
  | CaseZR(err, scrut, zrules) =>
    switch (zrules |> move_cursor_right_zrules) {
    | Some(zrules) => Some(CaseZR(err, scrut, zrules))
    | None =>
      Some(
        CursorE(
          OnDelim(1, Before),
          Case(err, scrut, zrules |> erase_zrules),
        ),
      )
    }
  | SubscriptZE1(err, ztarget, start_, end_) =>
    switch (move_cursor_right(ztarget)) {
    | Some(ztarget) => Some(SubscriptZE1(err, ztarget, start_, end_))
    | None =>
      Some(
        CursorE(
          OnDelim(0, Before),
          Subscript(err, erase(ztarget), start_, end_),
        ),
      )
    }
  | SubscriptZE2(err, target, zstart_, end_) =>
    switch (move_cursor_right(zstart_)) {
    | Some(zstart_) => Some(SubscriptZE2(err, target, zstart_, end_))
    | None =>
      Some(
        CursorE(
          OnDelim(1, Before),
          Subscript(err, target, erase(zstart_), end_),
        ),
      )
    }
  | SubscriptZE3(err, target, start_, zend_) =>
    switch (move_cursor_right(zend_)) {
    | Some(zend_) => Some(SubscriptZE3(err, target, start_, zend_))
    | None =>
      Some(
        CursorE(
          OnDelim(2, Before),
          Subscript(err, target, start_, erase(zend_)),
        ),
      )
    }
  | ApLivelitZ(llu, err, base_name, name, model, zsi) => {
      let (ty, ze) = ZSpliceInfo.prj_z(zsi);
      switch (move_cursor_left(ze)) {
      | Some(ze) =>
        let zsi = ZSpliceInfo.update_z(zsi, (ty, ze));
        Some(ApLivelitZ(llu, err, base_name, name, model, zsi));
      | None => None
      };
    }
and move_cursor_right_zrules =
  fun
  | (prefix, zrule, suffix) =>
    switch (move_cursor_right_zrule(zrule)) {
    | Some(zrule) => Some((prefix, zrule, suffix))
    | None =>
      switch (suffix) {
      | [] => None
      | [rule_after, ...new_suffix] =>
        Some((
          prefix @ [erase_zrule(zrule)],
          place_before_rule(rule_after),
          new_suffix,
        ))
      }
    }
and move_cursor_right_zrule =
  fun
  | z when is_after_zrule(z) => None
  | CursorR(OnOp(_), _) => None
  | CursorR(OnText(_), _) => None
  | CursorR(OnDelim(k, Before), rule) =>
    Some(CursorR(OnDelim(k, After), rule))
  | CursorR(OnDelim(k, After), Rule(p, clause)) =>
    // k == 0 || k == 1
    k == 0
      ? Some(RuleZP(ZPat.place_before(p), clause))
      : Some(RuleZE(p, place_before(clause)))
  | RuleZP(zp, clause) =>
    switch (ZPat.move_cursor_right(zp)) {
    | Some(zp) => Some(RuleZP(zp, clause))
    | None =>
      Some(CursorR(OnDelim(1, Before), Rule(ZPat.erase(zp), clause)))
    }
  | RuleZE(p, zclause) =>
    switch (move_cursor_right(zclause)) {
    | None => None
    | Some(zclause) => Some(RuleZE(p, zclause))
    };

let rec cursor_through_insts = ze => _cursor_inst_zblock(ze)
and _cursor_inst_zblock = ((_, zline, _)) => _cursor_inst_zline(zline)
and _cursor_inst_zline =
  fun
  | CursorL(_) => []
  | ExpLineZ(zopseq) => _cursor_inst_zopseq(zopseq)
  | LetLineZP(_)
  | LivelitDefLineZExpansionType(_)
  | LivelitDefLineZModelType(_)
  | LivelitDefLineZActionType(_) => []
  | LetLineZE(_, _, ze)
  | LivelitDefLineZCaptures({captures: ze, _})
  | LivelitDefLineZInit({init: ze, _})
  | LivelitDefLineZUpdate({update: ze, _})
  | LivelitDefLineZView({view: ze, _})
  | LivelitDefLineZShape({shape: ze, _})
  | LivelitDefLineZExpand({expand: ze, _}) => cursor_through_insts(ze)
and _cursor_inst_zopseq =
  fun
  | ZOpSeq(_, ZOperator(_)) => []
  | ZOpSeq(_, ZOperand(zoperand, _)) => _cursor_inst_zoperand(zoperand)
and _cursor_inst_zoperand =
  fun
  | CursorE(_, EmptyHole(u)) => [(TaggedNodeInstance.Hole, u)]
  | CursorE(_, ApLivelit(llu, _, _, _, _, _)) => [
      (TaggedNodeInstance.Livelit, llu),
    ]
  | CursorE(_)
  | LamZP(_) => []
  | LamZE(_, _, ze)
  | ParenthesizedZ(ze)
  | InjZ(_, _, ze)
  | CaseZE(_, ze, _)
  | SubscriptZE1(_, ze, _, _)
  | SubscriptZE2(_, _, ze, _)
  | SubscriptZE3(_, _, _, ze) => cursor_through_insts(ze)
  | ApLivelitZ(llu, _, _, _, _, zsplice_info) => [
      (TaggedNodeInstance.Livelit, llu),
      ...cursor_through_insts(ZSpliceInfo.prj_ze(zsplice_info)),
    ]
  | CaseZR(_, _, (_, zrule, _)) => _cursor_inst_zrule(zrule)
and _cursor_inst_zrule =
  fun
  | CursorR(_)
  | RuleZP(_) => []
  | RuleZE(_, ze) => cursor_through_insts(ze);

let cursor_on_inst = ze =>
  switch (List.rev(_cursor_inst_zblock(ze))) {
  | [] => None
  | [i, ..._] => Some(i)
  };
