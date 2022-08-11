open Virtual_dom.Vdom;
open Node;
open Util.Web;

let test_instance_view =
    (~font_metrics, (d, status): TestMap.test_instance_report) =>
  div(
    [clss(["test-instance", TestStatus.to_string(status)])],
    [Interface.dhcode_view(~font_metrics, ~width=40, d)],
  );

let jump_to_test = (~inject as _, _) => Event.Ignore;

let test_report_view =
    (
      ~inject,
      ~font_metrics,
      i: int,
      (_id, instance_reports): TestMap.test_report,
    ) => {
  let status =
    instance_reports |> TestMap.joint_status |> TestStatus.to_string;
  div(
    [Attr.class_("test-report"), Attr.on_click(jump_to_test(~inject))],
    [
      div(
        [clss(["test-id", "Test" ++ status])],
        // note: prints lexical index, not id
        [text(string_of_int(i + 1))],
      ),
      div(
        [Attr.class_("test-instances")],
        List.map(test_instance_view(~font_metrics), instance_reports),
      ),
    ],
  );
};

let test_reports_view = (~inject, ~font_metrics, test_map: TestMap.t) =>
  div(
    [clss(["panel-body", "test-reports"])],
    List.mapi(test_report_view(~inject, ~font_metrics), test_map),
  );

let test_bar_segment = (~inject, (_id, reports)) => {
  let status = reports |> TestMap.joint_status |> TestStatus.to_string;
  div(
    [clss(["segment", status]), Attr.on_click(jump_to_test(~inject))],
    [],
  );
};

let test_bar = (~inject, ~test_map: TestMap.t) =>
  div(
    [Attr.class_("test-bar")],
    List.map(test_bar_segment(~inject), test_map),
  );

let result_summary_str =
    (~n, ~p, ~q, ~n_str, ~ns_str, ~p_str, ~q_str, ~r_str): string => {
  let one_p = "one is " ++ p_str ++ " ";
  let one_q = "one is " ++ q_str ++ " ";
  let mny_p = Printf.sprintf("%d are %s ", p, p_str);
  let mny_q = Printf.sprintf("%d are %s ", q, q_str);
  let of_n = Printf.sprintf("Out of %d %s, ", n, ns_str);
  switch (n, p, q) {
  | (_, 0, 0) => "All " ++ ns_str ++ " " ++ r_str ++ "! "
  | (n, _, c) when n == c => "All " ++ ns_str ++ " " ++ q_str ++ " "
  | (n, f, _) when n == f => "All " ++ ns_str ++ " " ++ p_str ++ " "
  | (1, 0, 1) => "One " ++ n_str ++ " " ++ q_str ++ " "
  | (1, 1, 0) => "One " ++ n_str ++ " " ++ p_str ++ " "
  | (2, 1, 1) =>
    "One " ++ n_str ++ " " ++ p_str ++ " and one " ++ q_str ++ " "
  | (_, 0, 1) => of_n ++ one_q
  | (_, 1, 0) => of_n ++ one_p
  | (_, 1, 1) => of_n ++ one_p ++ "and " ++ one_q
  | (_, 1, _) => of_n ++ one_p ++ "and " ++ mny_q
  | (_, _, 1) => of_n ++ mny_p ++ "and " ++ one_q
  | (_, 0, _) => of_n ++ mny_q
  | (_, _, 0) => of_n ++ mny_p
  | (_, _, _) => of_n ++ mny_p ++ "and " ++ mny_q
  };
};

/*
 let test_summary_str = (~test_map: TestMap.t): string => {
   let total = TestMap.count(test_map);
   let failing = TestMap.count_status(Fail, test_map);
   let unfinished = TestMap.count_status(Indet, test_map);
   let one_failing = "one is failing ";
   let one_unfinished = "one is unfinished ";
   let mny_failing = Printf.sprintf("%d are failing ", failing);
   let mny_unfinished = Printf.sprintf("%d are unfinished ", unfinished);
   let of_n_tests = Printf.sprintf("Out of %d tests, ", total);
   switch (total, failing, unfinished) {
   | (_, 0, 0) => "All tests passing! "
   | (n, _, c) when n == c => "All tests unfinished "
   | (n, f, _) when n == f => "All tests failing "
   | (1, 0, 1) => "One test unfinished "
   | (1, 1, 0) => "One test failing "
   | (2, 1, 1) => "One test failing and one unfinished "
   | (_, 0, 1) => of_n_tests ++ one_unfinished
   | (_, 1, 0) => of_n_tests ++ one_failing
   | (_, 1, 1) => of_n_tests ++ one_failing ++ "and " ++ one_unfinished
   | (_, 1, _) => of_n_tests ++ one_failing ++ "and " ++ mny_unfinished
   | (_, _, 1) => of_n_tests ++ mny_failing ++ "and " ++ one_unfinished
   | (_, 0, _) => of_n_tests ++ mny_unfinished
   | (_, _, 0) => of_n_tests ++ mny_failing
   | (_, _, _) => of_n_tests ++ mny_failing ++ "and " ++ mny_unfinished
   };
 };
 */

let test_summary_str = (~test_map: TestMap.t): string =>
  result_summary_str(
    ~n=TestMap.count(test_map),
    ~p=TestMap.count_status(Fail, test_map),
    ~q=TestMap.count_status(Indet, test_map),
    ~n_str="test",
    ~ns_str="tests",
    ~p_str="failing",
    ~q_str="unfinished",
    ~r_str="passing",
  );

let percent_view = (n: int, p: int): Node.t => {
  let percentage = 100. *. float_of_int(p) /. float_of_int(n);
  div(
    [clss(["test-percent", n == p ? "all-pass" : "some-fail"])],
    [text(Printf.sprintf("%.0f%%", percentage))],
  );
};

let test_percentage = (test_map: TestMap.t): Node.t => {
  let total = TestMap.count(test_map);
  let passing = TestMap.count_status(Pass, test_map);
  percent_view(total, passing);
};

let test_text = (test_map: TestMap.t): Node.t =>
  div(
    [Attr.class_("test-text")],
    [
      test_percentage(test_map),
      div([], [text(":")]),
      text(test_summary_str(~test_map)),
    ],
  );

let test_summary = (~inject, ~test_map) => {
  let failing = TestMap.count_status(Fail, test_map);
  let unfinished = TestMap.count_status(Indet, test_map);
  let status_class =
    switch (failing, unfinished) {
    | (0, 0) => "Pass"
    | (0, _) => "Indet"
    | _ => "Fail"
    };
  div(
    [clss(["test-summary", "instructional-msg", status_class])],
    [test_text(test_map), test_bar(~inject, ~test_map)],
  );
};

let view_of_main_title_bar = (title_text: string) =>
  div([clss(["title-bar", "panel-title-bar"])], [Node.text(title_text)]);

let view =
    (
      ~title: string,
      ~inject=(),
      ~font_metrics,
      d: Elaborator_Exp.ElaborationResult.t,
    )
    : t =>
  switch (Interface.get_result(d)) {
  | None => div([], [])
  | Some((_, test_map)) =>
    div_if(
      test_map != [],
      [clss(["panel", "test-panel"])],
      [
        view_of_main_title_bar(title),
        test_reports_view(~inject, ~font_metrics, test_map),
        test_summary(~inject, ~test_map),
      ],
    )
  };

let inspector_view =
    (~inject as _, ~font_metrics, ~test_map: TestMap.t, id: int): option(t) => {
  switch (TestMap.lookup(id, test_map)) {
  | Some(instances) when TestMap.joint_status(instances) != Indet =>
    Some(
      div(
        [Attr.class_("test-inspector")],
        [
          div(
            [Attr.class_("test-instances")],
            List.map(test_instance_view(~font_metrics), instances),
          ),
        ],
      ),
    )
  | _ => None
  };
};