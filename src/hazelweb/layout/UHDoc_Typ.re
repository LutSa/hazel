let inline_padding_of_operator:
  UHTyp.operator => (UHDoc_common.t, UHDoc_common.t) =
  fun
  | Prod => (UHDoc_common.empty_, UHDoc_common.space_)
  | Arrow
  | Sum => (UHDoc_common.space_, UHDoc_common.space_);

let mk_EmptyHole: string => UHDoc_common.t =
  UHDoc_common.mk_EmptyHole(~sort=Typ);
let mk_Parenthesized: UHDoc_common.formatted_child => UHDoc_common.t =
  UHDoc_common.mk_Parenthesized(~sort=Typ);
let mk_NTuple:
  (
    ~mk_operand: (~enforce_inline: bool, ~map: AssertMap.t, 'a) =>
                 UHDoc_common.t,
    ~mk_operator: UHTyp.operator => UHDoc_common.t,
    ~enforce_inline: bool,
    ~map: AssertMap.t,
    OpSeq.t('a, UHTyp.operator)
  ) =>
  UHDoc_common.t =
  UHDoc_common.mk_NTuple(
    ~sort=Typ,
    ~get_tuple_elements=UHTyp.get_prod_elements,
    ~inline_padding_of_operator,
  );

let rec mk =
  lazy(
    UHDoc_common.memoize(
      (~memoize: bool, ~enforce_inline: bool, ~map: AssertMap.t, uty: UHTyp.t) =>
      (
        Lazy.force(mk_opseq, ~memoize, ~enforce_inline, ~map, uty): UHDoc_common.t
      )
    )
  )
and mk_opseq =
  lazy(
    UHDoc_common.memoize(
      (
        ~memoize: bool,
        ~enforce_inline: bool,
        ~map: AssertMap.t,
        opseq: UHTyp.opseq,
      ) =>
      (
        mk_NTuple(
          ~mk_operand=Lazy.force(mk_operand, ~memoize),
          ~mk_operator,
          ~enforce_inline,
          ~map,
          opseq,
        ): UHDoc_common.t
      )
    )
  )
and mk_operator = (op: UHTyp.operator): UHDoc_common.t =>
  UHDoc_common.mk_op(Operators_Typ.to_string(op))
and mk_operand =
  lazy(
    UHDoc_common.memoize(
      (
        ~memoize: bool,
        ~enforce_inline: bool,
        ~map: AssertMap.t,
        operand: UHTyp.operand,
      ) =>
      (
        switch (operand) {
        | Hole =>
          print_endline(string_of_bool(map == map));
          mk_EmptyHole("?");
        | Unit => UHDoc_common.mk_Unit()
        | Int => UHDoc_common.mk_Int()
        | Float => UHDoc_common.mk_Float()
        | Bool => UHDoc_common.mk_Bool()
        | Parenthesized(body) =>
          let body = mk_child(~memoize, ~enforce_inline, ~child_step=0, body);
          mk_Parenthesized(body);
        | List(body) =>
          let body = mk_child(~memoize, ~enforce_inline, ~child_step=0, body);
          UHDoc_common.mk_List(body);
        }: UHDoc_common.t
      )
    )
  )
and mk_child =
    (~memoize: bool, ~enforce_inline: bool, ~child_step: int, uty: UHTyp.t)
    : UHDoc_common.formatted_child => {
  let formattable = (~enforce_inline: bool) =>
    Lazy.force(mk, ~memoize, ~enforce_inline, ~map=AssertMap.empty, uty)
    |> UHDoc_common.annot_Step(child_step);
  enforce_inline
    ? EnforcedInline(formattable(~enforce_inline=true))
    : Unformatted(formattable);
};
