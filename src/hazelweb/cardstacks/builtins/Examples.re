let just_hole: UHExp.t = UHExp.Block.wrap(EmptyHole(0));

let holey_lambda: UHExp.t = {
  let lam =
    UHExp.(
      Parenthesized(
        Block.wrap(
          lam(
            OpSeq.wrap(UHPat.EmptyHole(0)),
            Block.wrap(UHExp.EmptyHole(1)),
          ),
        ),
      )
    );
  let arg = UHExp.EmptyHole(2);
  UHExp.Block.wrap'(
    Seq.mk(lam, [(Operators_Exp.Space, arg)]) |> UHExp.mk_OpSeq,
  );
};

let let_line: UHExp.t =
  UHExp.[
    letline(OpSeq.wrap(UHPat.var("y")), Block.wrap(EmptyHole(0))),
    EmptyLine,
    letline(OpSeq.wrap(UHPat.var("x")), Block.wrap(EmptyHole(1))),
    ExpLine(var("x") |> OpSeq.wrap),
    ExpLine(var("y") |> OpSeq.wrap),
  ];

let map_example: UHExp.t = {
  let case_node =
    UHExp.(
      case(
        Block.wrap(var("xs")),
        [
          Rule(OpSeq.wrap(UHPat.listnil()), Block.wrap(listnil())),
          Rule(
            UHPat.(
              Seq.mk(var("y"), [(Operators_Pat.Cons, var("ys"))])
              |> mk_OpSeq
            ),
            Operators_Exp.(
              Block.wrap'(
                Seq.mk(
                  Parenthesized(
                    Block.wrap'(
                      Seq.mk(var("f"), [(Space, var("y"))]) |> mk_OpSeq,
                    ),
                  ),
                  [
                    (
                      Cons,
                      Parenthesized(
                        Block.wrap'(
                          Seq.mk(
                            var("map"),
                            [(Space, var("f")), (Space, var("ys"))],
                          )
                          |> mk_OpSeq,
                        ),
                      ),
                    ),
                  ],
                )
                |> mk_OpSeq,
              )
            ),
          ),
        ],
      )
    );
  let lam_node =
    UHExp.(
      lam(
        OpSeq.wrap(UHPat.var("f")),
        Block.wrap(
          lam(OpSeq.wrap(UHPat.var("xs")), Block.wrap(case_node)),
        ),
      )
    );
  let letline_node =
    UHExp.(
      letline(
        OpSeq.wrap(
          UHPat.TypeAnn(
            NotInHole,
            UHPat.var("map"),
            Operators_Typ.(
              UHTyp.(
                Seq.mk(
                  Parenthesized(Seq.mk(Int, [(Arrow, Int)]) |> mk_OpSeq),
                  [
                    (Arrow, List(OpSeq.wrap(Int))),
                    (Arrow, List(OpSeq.wrap(Int))),
                  ],
                )
                |> mk_OpSeq
              )
            ),
          ),
        ),
        Block.wrap(lam_node),
      )
    );
  UHExp.[letline_node, ExpLine(EmptyHole(0) |> OpSeq.wrap)];
};

let qsort_example: UHExp.t = {
  let append_case =
    UHExp.(
      case(
        Block.wrap(var("xs")),
        [
          Rule(OpSeq.wrap(UHPat.listnil()), Block.wrap(var("ys"))),
          Rule(
            UHPat.(
              Seq.mk(var("z"), [(Operators_Pat.Cons, var("zs"))])
              |> mk_OpSeq
            ),
            Operators_Exp.(
              Block.wrap'(
                Seq.mk(
                  var("z"),
                  [
                    (
                      Cons,
                      Parenthesized(
                        Block.wrap'(
                          Seq.mk(
                            var("append"),
                            [(Space, var("zs")), (Space, var("ys"))],
                          )
                          |> mk_OpSeq,
                        ),
                      ),
                    ),
                  ],
                )
                |> mk_OpSeq,
              )
            ),
          ),
        ],
      )
    );
  let append_lam =
    UHExp.(
      lam(
        OpSeq.wrap(UHPat.var("xs")),
        Block.wrap(
          lam(OpSeq.wrap(UHPat.var("ys")), Block.wrap(append_case)),
        ),
      )
    );
  let append_letline =
    UHExp.(
      letline(
        OpSeq.wrap(
          UHPat.TypeAnn(
            NotInHole,
            UHPat.var("append"),
            Operators_Typ.(
              Seq.mk(
                UHTyp.List(OpSeq.wrap(UHTyp.Int)),
                [
                  (Arrow, List(OpSeq.wrap(UHTyp.Int))),
                  (Arrow, List(OpSeq.wrap(UHTyp.Int))),
                ],
              )
              |> UHTyp.mk_OpSeq
            ),
          ),
        ),
        Block.wrap(append_lam),
      )
    );
  let partition_case =
    UHExp.(
      Operators_Exp.(
        case(
          Block.wrap(var("xs")),
          [
            Rule(
              OpSeq.wrap(UHPat.listnil()),
              Block.wrap(
                Parenthesized(
                  Block.wrap'(
                    Seq.mk(listnil(), [(Comma, listnil())]) |> mk_OpSeq,
                  ),
                ),
              ),
            ),
            Rule(
              UHPat.(
                Seq.mk(var("y"), [(Operators_Pat.Cons, var("ys"))])
                |> mk_OpSeq
              ),
              [
                letline(
                  UHPat.(
                    OpSeq.wrap(
                      Parenthesized(
                        Seq.mk(
                          var("ys1"),
                          [(Operators_Pat.Comma, var("ys2"))],
                        )
                        |> mk_OpSeq,
                      ),
                    )
                  ),
                  Block.wrap'(
                    Seq.mk(
                      var("partition"),
                      [(Space, var("f")), (Space, var("ys"))],
                    )
                    |> mk_OpSeq,
                  ),
                ),
                ExpLine(
                  case(
                    Block.wrap'(
                      Seq.mk(var("f"), [(Space, var("y"))]) |> mk_OpSeq,
                    ),
                    [
                      Rule(
                        OpSeq.wrap(UHPat.boollit(true)),
                        Block.wrap(
                          Parenthesized(
                            Block.wrap'(
                              Seq.mk(
                                var("y"),
                                [(Cons, var("ys1")), (Comma, var("ys2"))],
                              )
                              |> mk_OpSeq,
                            ),
                          ),
                        ),
                      ),
                      Rule(
                        OpSeq.wrap(UHPat.boollit(false)),
                        Block.wrap(
                          Parenthesized(
                            Block.wrap'(
                              Seq.mk(
                                var("ys1"),
                                [(Comma, var("y")), (Cons, var("ys2"))],
                              )
                              |> mk_OpSeq,
                            ),
                          ),
                        ),
                      ),
                    ],
                  )
                  |> OpSeq.wrap,
                ),
              ],
            ),
          ],
        )
      )
    );
  let partition_lam =
    UHExp.(
      lam(
        OpSeq.wrap(UHPat.var("f")),
        Block.wrap(
          lam(OpSeq.wrap(UHPat.var("xs")), Block.wrap(partition_case)),
        ),
      )
    );
  let partition_letline =
    UHExp.(
      letline(
        OpSeq.wrap(
          UHPat.TypeAnn(
            NotInHole,
            UHPat.var("partition"),
            UHTyp.(
              Operators_Typ.(
                Seq.mk(
                  Parenthesized(Seq.mk(Int, [(Arrow, Bool)]) |> mk_OpSeq),
                  [
                    (Arrow, List(OpSeq.wrap(Int))),
                    (
                      Arrow,
                      Parenthesized(
                        Seq.mk(
                          List(OpSeq.wrap(Int)),
                          [(Prod, List(OpSeq.wrap(Int)))],
                        )
                        |> mk_OpSeq,
                      ),
                    ),
                  ],
                )
                |> mk_OpSeq
              )
            ),
          ),
        ),
        Block.wrap(partition_lam),
      )
    );

  let qsort_line =
    UHExp.(
      Operators_Exp.(
        ExpLine(
          Seq.mk(
            var("qsort"),
            [
              (
                Space,
                Parenthesized(
                  Block.wrap'(
                    Seq.mk(
                      intlit("4"),
                      [
                        (Cons, intlit("2")),
                        (Cons, intlit("6")),
                        (Cons, intlit("5")),
                        (Cons, intlit("3")),
                        (Cons, intlit("1")),
                        (Cons, intlit("7")),
                        (Cons, listnil()),
                      ],
                    )
                    |> mk_OpSeq,
                  ),
                ),
              ),
            ],
          )
          |> mk_OpSeq,
        )
      )
    );

  UHExp.[append_letline, EmptyLine, partition_letline, EmptyLine, qsort_line];
};

let rec qsort_n = (n: int): UHExp.t =>
  if (n == 0) {
    [];
  } else {
    [
      UHExp.letline(
        OpSeq.wrap(UHPat.var("qsort" ++ Int.to_string(n))),
        qsort_example,
      ),
      ...qsort_n(n - 1),
    ];
  };

let assistant_test =
  {|((ExpLine(OpSeq(Placeholder 0)(S(Parenthesized((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 0)E)))))E)))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole isBuddo)(OpSeq(Placeholder 0)(S Bool E)))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 1)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole poggleHeft)(OpSeq(Placeholder 0)(S Float E)))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 2)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole numFleens)(OpSeq(Placeholder 0)(S Int E)))E))((ExpLine(OpSeq(Placeholder 0)(S(IntLit NotInHole 6)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole flarfus)(OpSeq(Placeholder 0)(S(Parenthesized(OpSeq(BinOp NotInHole Prod(Placeholder 0)(Placeholder 1))(S Bool(A Prod(S Bool E)))))E)))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 3)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole toogles)(OpSeq(Placeholder 0)(S(List(OpSeq(Placeholder 0)(S Int E)))E)))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 4)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole intToFloat)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(Placeholder 1))(S Int(A Arrow(S Float E)))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 5)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole floatToInt)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(Placeholder 1))(S Float(A Arrow(S Int E)))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 6)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole fleem)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(Placeholder 1))(S Hole(A Arrow(S Int E)))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 7)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole blom)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(Placeholder 1))(S Float(A Arrow(S Int E)))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 8)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole blogger)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(BinOp NotInHole Arrow(Placeholder 1)(Placeholder 2)))(S Int(A Arrow(S Hole(A Arrow(S Bool E)))))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 9)E)))))(LetLine(OpSeq(Placeholder 0)(S(TypeAnn NotInHole(Var NotInHole NotInVarHole grackle)(OpSeq(BinOp NotInHole Arrow(Placeholder 0)(BinOp NotInHole Arrow(Placeholder 1)(Placeholder 2)))(S Float(A Arrow(S(List(OpSeq(Placeholder 0)(S Bool E)))(A Arrow(S Int E)))))))E))((ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 10)E)))))(ExpLine(OpSeq(BinOp NotInHole FPlus(Placeholder 0)(Placeholder 1))(S(Parenthesized((ExpLine(OpSeq(BinOp(InHole TypeInconsistent 515)Plus(Placeholder 0)(Placeholder 1))(S(IntLit NotInHole 111)(A Plus(S(Parenthesized((ExpLine(OpSeq(BinOp NotInHole Space(Placeholder 0)(Placeholder 1))(S(Var NotInHole NotInVarHole floatToInt)(A Space(S(IntLit(InHole TypeInconsistent 514)222)E)))))))E)))))))(A FPlus(S(Var(InHole TypeInconsistent 516)NotInVarHole numFleens)E)))))(ExpLine(OpSeq(BinOp NotInHole Times(Placeholder 0)(Placeholder 1))(S(IntLit NotInHole 111)(A Times(S(Parenthesized((ExpLine(OpSeq(BinOp NotInHole Space(Placeholder 0)(Placeholder 1))(S(Var NotInHole NotInVarHole fleem)(A Space(S(Parenthesized((ExpLine(OpSeq(BinOp NotInHole Space(Placeholder 0)(Placeholder 1))(S(Var NotInHole NotInVarHole blom)(A Space(S(IntLit(InHole TypeInconsistent 517)4)E)))))))E)))))))E)))))(ExpLine(OpSeq(BinOp NotInHole FPlus(Placeholder 0)(Placeholder 1))(S(IntLit(InHole TypeInconsistent 717)1)(A FPlus(S(Parenthesized((ExpLine(OpSeq(BinOp(InHole TypeInconsistent 719)Space(Placeholder 0)(Placeholder 1))(S(Var NotInHole NotInVarHole blom)(A Space(S(IntLit(InHole TypeInconsistent 718)4)E)))))))E)))))(ExpLine(OpSeq(Placeholder 0)(S(EmptyHole 21)E))))|}
  |> Sexplib.Sexp.of_string
  |> UHExp.t_of_sexp;

let inconsistent_branches: UHExp.t =
  UHExp.(
    Block.wrap(
      case(
        ~err=InconsistentBranches([Bool, Float, Float], 0),
        Block.wrap(UHExp.IntLit(NotInHole, "1")),
        [
          Rule(
            OpSeq.wrap(UHPat.IntLit(NotInHole, "0")),
            Block.wrap(UHExp.BoolLit(NotInHole, true)),
          ),
          Rule(
            OpSeq.wrap(UHPat.IntLit(NotInHole, "1")),
            Block.wrap(UHExp.FloatLit(NotInHole, "1.")),
          ),
          Rule(
            OpSeq.wrap(UHPat.IntLit(NotInHole, "2")),
            Block.wrap(UHExp.FloatLit(NotInHole, "2.")),
          ),
        ],
      ),
    )
  );

let examples = [
  ("hole", just_hole),
  ("lambda", holey_lambda),
  ("let", let_line),
  ("map", map_example),
  ("quicksort", qsort_example),
  ("assistant", assistant_test),
  ("inconsistent branches", inconsistent_branches),
];

let example_to_card = ((name: string, e: UHExp.t)): CardInfo.t => {
  name,
  caption: Virtual_dom.Vdom.Node.div([], []),
  init_zexp: ZExp.place_before(e),
};

let cardstack: CardstackInfo.t = {
  title: "examples",
  cards: List.map(example_to_card, examples),
};

let tests = [
  ("quicksort x1", qsort_n(1)),
  ("quicksort x10", qsort_n(10)),
  ("quicksort x100", qsort_n(100)),
];

let teststack: CardstackInfo.t = {
  title: "tests",
  cards: List.map(example_to_card, tests),
};