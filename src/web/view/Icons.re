open Virtual_dom.Vdom;

let icon_size = 20.;

let simple_icon = (~transform="", ~view: string, ds: list(string)) =>
  /* takes a list of paths as strings, a viewport as a string,
     and an optional (string) transform to apply to each */
  Node.create_svg(
    "svg",
    Attr.[
      create("viewBox", view),
      create("width", Printf.sprintf("%fpx", icon_size)),
      create("height", Printf.sprintf("%fpx", icon_size)),
      create("preserveAspectRatio", "none"),
    ],
    List.map(
      d =>
        Node.create_svg(
          "path",
          [Attr.create("d", d)]
          @ (transform == "" ? [] : [Attr.create("transform", transform)]),
          [],
        ),
      ds,
    ),
  );

let export =
  simple_icon(
    ~view="0 0 67.671 67.671",
    [
      "M52.946,23.348H42.834v6h10.112c3.007,0,5.34,1.536,5.34,2.858v26.606c0,1.322-2.333,2.858-5.34,2.858H14.724   c-3.007,0-5.34-1.536-5.34-2.858V32.207c0-1.322,2.333-2.858,5.34-2.858h10.11v-6h-10.11c-6.359,0-11.34,3.891-11.34,8.858v26.606   c0,4.968,4.981,8.858,11.34,8.858h38.223c6.358,0,11.34-3.891,11.34-8.858V32.207C64.286,27.239,59.305,23.348,52.946,23.348z",
      "M24.957,14.955c0.768,0,1.535-0.293,2.121-0.879l3.756-3.756v13.028v6v11.494c0,1.657,1.343,3,3,3s3-1.343,3-3V29.348v-6   V10.117l3.959,3.959c0.586,0.586,1.354,0.879,2.121,0.879s1.535-0.293,2.121-0.879c1.172-1.171,1.172-3.071,0-4.242l-8.957-8.957   C35.492,0.291,34.725,0,33.958,0c-0.008,0-0.015,0-0.023,0s-0.015,0-0.023,0c-0.767,0-1.534,0.291-2.12,0.877l-8.957,8.957   c-1.172,1.171-1.172,3.071,0,4.242C23.422,14.662,24.189,14.955,24.957,14.955z",
    ],
  );

let undo =
  simple_icon(
    ~view="0 0 512 512",
    [
      "M129.7,46.4l37.2,37.7l-66.6,67.1h254.4c86.8,0,157.2,70.4,157.2,157.2s-70.4,157.2-157.2,157.2h-52.4v-52.4h52.4 c57.9,0,104.8-46.9,104.8-104.8s-46.9-104.8-104.8-104.8H100.4l66.6,65.8l-37.2,36.9L0,177.4L129.7,46.4z",
    ],
  );

let redo =
  simple_icon(
    ~view="0 0 512 512",
    [
      "M382.3,46.4l-37.2,37.7l66.6,67.1H157.2C70.4,151.2,0,221.6,0,308.4s70.4,157.2,157.2,157.2h52.4v-52.4h-52.4 c-57.9,0-104.8-46.9-104.8-104.8s46.9-104.8,104.8-104.8h254.4l-66.6,65.8l36.9,36.9l130-128.9L382.3,46.4z",
    ],
  );

let circle_question =
  simple_icon(
    ~view="-0.5 -0.5 25 25",
    [
      "M12 2c5.514 0 10 4.486 10 10s-4.486 10-10 10-10-4.486-10-10 4.486-10 10-10zm0-2c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12zm1.25 17c0 .69-.559 1.25-1.25 1.25-.689 0-1.25-.56-1.25-1.25s.561-1.25 1.25-1.25c.691 0 1.25.56 1.25 1.25zm1.393-9.998c-.608-.616-1.515-.955-2.551-.955-2.18 0-3.59 1.55-3.59 3.95h2.011c0-1.486.829-2.013 1.538-2.013.634 0 1.307.421 1.364 1.226.062.847-.39 1.277-.962 1.821-1.412 1.343-1.438 1.993-1.432 3.468h2.005c-.013-.664.03-1.203.935-2.178.677-.73 1.519-1.638 1.536-3.022.011-.924-.284-1.719-.854-2.297z",
    ],
  );

let github =
  simple_icon(
    ~view="-0.5 -0.5 25 25",
    [
      "M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z",
    ],
  );

let back =
  simple_icon(
    ~view="0 0 330 330",
    [
      "M250.606,154.389l-150-149.996c-5.857-5.858-15.355-5.858-21.213,0.001  c-5.857,5.858-5.857,15.355,0.001,21.213l139.393,139.39L79.393,304.394c-5.857,5.858-5.857,15.355,0.001,21.213  C82.322,328.536,86.161,330,90,330s7.678-1.464,10.607-4.394l149.999-150.004c2.814-2.813,4.394-6.628,4.394-10.606  C255,161.018,253.42,157.202,250.606,154.389z",
    ],
    ~transform="scale(-0.75, 0.75) translate(-330, 50)",
  );

let forward =
  simple_icon(
    ~view="0 0 330 330",
    [
      "M250.606,154.389l-150-149.996c-5.857-5.858-15.355-5.858-21.213,0.001  c-5.857,5.858-5.857,15.355,0.001,21.213l139.393,139.39L79.393,304.394c-5.857,5.858-5.857,15.355,0.001,21.213  C82.322,328.536,86.161,330,90,330s7.678-1.464,10.607-4.394l149.999-150.004c2.814-2.813,4.394-6.628,4.394-10.606  C255,161.018,253.42,157.202,250.606,154.389z",
    ],
    ~transform="scale(0.75, 0.75) translate(0, 50)",
  );

let eye =
  simple_icon(
    ~view="0 0 48 48",
    [
      "M24 9c-10 0-18.54 6.22-22 15 3.46 8.78 12 15 22 15 10.01 0 18.54-6.22 22-15-3.46-8.78-11.99-15-22-15zm0 25c-5.52 0-10-4.48-10-10s4.48-10 10-10 10 4.48 10 10-4.48 10-10 10zm0-16c-3.31 0-6 2.69-6 6s2.69 6 6 6 6-2.69 6-6-2.69-6-6-6z",
    ],
  );

let trash =
  simple_icon(
    ~view="0 0 24 24",
    [
      "M3 6v18h18v-18h-18zm5 14c0 .552-.448 1-1 1s-1-.448-1-1v-10c0-.552.448-1 1-1s1 .448 1 1v10zm5 0c0 .552-.448 1-1 1s-1-.448-1-1v-10c0-.552.448-1 1-1s1 .448 1 1v10zm5 0c0 .552-.448 1-1 1s-1-.448-1-1v-10c0-.552.448-1 1-1s1 .448 1 1v10zm4-18v2h-20v-2h5.711c.9 0 1.631-1.099 1.631-2h5.315c0 .901.73 2 1.631 2h5.712z",
    ],
  );

let hazelnut =
  simple_icon(
    ~view="100 0 500 500",
    [
      "m499.84 130.51c-93.363-93.363-207.59-129.06-300.65-36.051-95.07 95.113-116.07 213.15-62.562 350.83 3.1484 8.0508-0.875 28.438 9.5391 38.895 10.414 10.457 29.836 5.4688 36.707 8.0078 128.84 47.512 254.8 37.188 353.02-61.25 93.008-92.797 57.352-207.03-36.055-300.43zm-12.383 12.383c32.289 32.289 88.508 99.574 85.094 173.64-76.211-33.34-85.922-85.141-144.77-143.98-35.48-35.48-103.82-58.055-116.07-114.62 66.02-4.1992 129.24 38.324 175.74 84.961zm36.051 275.89c-87.5 87.5-199.98 106.62-334.55 56.875-9.1875-3.3672-27.344-1.0508-30.625-3.9375-3.8945-3.9375-1.5742-22.488-5.6016-32.855-51.539-132.48-32.375-241.11 58.625-332.11 25.418-25.375 53.113-40.949 82.773-46.68 13.434 66.012 87.195 90.777 121.5 124.86 58.449 58.672 69.082 114.23 154.88 149.71-5.0742 27.91-19.25 56.438-46.988 84.129z",
      "m274.01 374.72 108.11-108.11c1.7695-1.6133 2.8047-3.8789 2.8594-6.2734 0.054688-2.3945-0.875-4.707-2.5703-6.3984-1.6953-1.6914-4.0117-2.6133-6.4062-2.5508-2.3945 0.066406-4.6602 1.1055-6.2656 2.8828l-108.11 108.06c-3.418 3.4219-3.418 8.9648 0 12.383s8.9609 3.418 12.383 0z",
      "m445.42 399.61c-0.52344 0.39453-52.938 40.555-115.54 46.242-4.832 0.4375-8.3984 4.707-7.9609 9.5391 0.43359 4.832 4.7031 8.3984 9.5352 7.9609 67.855-6.168 122.5-48.125 124.69-49.832 1.9062-1.3945 3.168-3.5 3.4961-5.8398 0.32813-2.3398-0.30469-4.7109-1.75-6.5781-1.4492-1.8672-3.5898-3.0703-5.9375-3.3359-2.3438-0.26172-4.6992 0.43359-6.5273 1.9297z",
      "m344.93 410.64c2.2617 4.2617 7.5469 5.8828 11.812 3.6328 34.762-18.434 68.016-39.586 99.441-63.262 3.8086-2.9844 4.4727-8.4883 1.4883-12.297-2.9844-3.8047-8.4883-4.4688-12.293-1.4844-30.648 23.004-63.031 43.598-96.863 61.598-4.2422 2.2812-5.8477 7.5586-3.5859 11.812z",
      "m350 357.61c-2.8438-3.8984-8.3008-4.7578-12.207-1.9258-0.52344 0.35156-50.18 36.445-105.61 71.094-2.0391 1.1953-3.5078 3.1641-4.0742 5.4609-0.5625 2.2969-0.17578 4.7227 1.0781 6.7305 1.2539 2.0039 3.2656 3.4141 5.5781 3.9141 2.3125 0.49609 4.7266 0.039062 6.6953-1.2734 56.043-34.738 106-71.402 106.53-71.793 3.9258-2.8164 4.8242-8.2812 2.0117-12.207z",
    ],
  );