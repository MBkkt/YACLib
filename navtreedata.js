/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "YACLib", "index.html", [
    [ "Table of Contents", "index.html#autotoc_md1", null ],
    [ "About YACLib", "index.html#autotoc_md2", null ],
    [ "Getting started", "index.html#autotoc_md3", null ],
    [ "Examples", "index.html#autotoc_md4", null ],
    [ "Requirements", "index.html#autotoc_md15", [
      [ "Operating systems", "index.html#autotoc_md16", null ],
      [ "Compilers", "index.html#autotoc_md17", null ],
      [ "C++ standard", "index.html#autotoc_md18", null ],
      [ "Build systems", "index.html#autotoc_md19", null ]
    ] ],
    [ "Releases", "index.html#autotoc_md20", null ],
    [ "Contributing", "index.html#autotoc_md21", null ],
    [ "Contacts", "index.html#autotoc_md22", null ],
    [ "License", "index.html#autotoc_md23", null ],
    [ "Dependency", "md_doc_dependency.html", [
      [ "<a href=\"https://github.com/google/googletest\">Google Test</a>", "md_doc_dependency.html#autotoc_md25", [
        [ "Asynchronous pipeline", "index.html#autotoc_md5", null ],
        [ "Thread Pool", "index.html#autotoc_md6", null ],
        [ "Serial Executor, Strand, Async Mutex", "index.html#autotoc_md7", null ],
        [ "WhenAll", "index.html#autotoc_md8", null ],
        [ "WhenAny", "index.html#autotoc_md9", null ],
        [ "Future unwrapping", "index.html#autotoc_md10", null ],
        [ "Timed wait", "index.html#autotoc_md11", null ],
        [ "Exception recovering", "index.html#autotoc_md12", null ],
        [ "Error recovering", "index.html#autotoc_md13", null ],
        [ "Use Result for smart recovering", "index.html#autotoc_md14", null ],
        [ "Version", "md_doc_dependency.html#autotoc_md26", null ],
        [ "Why do we need it?", "md_doc_dependency.html#autotoc_md27", null ]
      ] ],
      [ "<a href=\"https://github.com/google/benchmark\">Google Benchmark</a>", "md_doc_dependency.html#autotoc_md28", [
        [ "Version", "md_doc_dependency.html#autotoc_md29", null ]
      ] ]
    ] ],
    [ "Design", "md_doc_design.html", [
      [ "Rationale:", "md_doc_design.html#autotoc_md31", [
        [ "Easy to use:", "md_doc_design.html#autotoc_md49", null ],
        [ "Zero cost abstraction:", "md_doc_design.html#autotoc_md51", null ],
        [ "Easy to build:", "md_doc_design.html#autotoc_md52", null ],
        [ "Good test coverage:", "md_doc_design.html#autotoc_md54", null ]
      ] ]
    ] ],
    [ "Дизайн", "md_doc_design_ru.html", [
      [ "Мотивация:", "md_doc_design_ru.html#autotoc_md33", [
        [ "Easy to use:", "md_doc_design_ru.html#autotoc_md34", null ],
        [ "Zero cost abstraction:", "md_doc_design_ru.html#autotoc_md35", null ],
        [ "Easy to build:", "md_doc_design_ru.html#autotoc_md36", null ],
        [ "Good test coverage:", "md_doc_design_ru.html#autotoc_md37", null ]
      ] ],
      [ "Что уже сделано:", "md_doc_design_ru.html#autotoc_md38", [
        [ "Были написаны следующие абстракции:", "md_doc_design_ru.html#autotoc_md39", null ],
        [ "Future/Promise/Run abstraction", "md_doc_design_ru.html#autotoc_md40", null ],
        [ "WhenAll Combinator", "md_doc_design_ru.html#autotoc_md41", null ],
        [ "ThreadFactory", "md_doc_design_ru.html#autotoc_md42", null ]
      ] ],
      [ "Что планируется сделать", "md_doc_design_ru.html#autotoc_md43", [
        [ "Lazy Future", "md_doc_design_ru.html#autotoc_md44", null ]
      ] ],
      [ "Разные реализации ThreadPool и их бенчмарки", "md_doc_design_ru.html#autotoc_md45", [
        [ "План реализации:", "md_doc_design_ru.html#autotoc_md46", null ]
      ] ],
      [ "Fibers", "md_doc_design_ru.html#autotoc_md47", [
        [ "План реализации:", "md_doc_design_ru.html#autotoc_md48", null ]
      ] ],
      [ "Concurrent algorithms", "md_doc_design_ru.html#autotoc_md50", null ],
      [ "Почему я должен использовать YACLib?", "md_doc_design_ru.html#autotoc_md53", [
        [ "STL", "md_doc_design_ru.html#autotoc_md55", null ],
        [ "OpenMP", "md_doc_design_ru.html#autotoc_md56", null ],
        [ "oneAPI TBB", "md_doc_design_ru.html#autotoc_md57", null ],
        [ "Boost.Asio", "md_doc_design_ru.html#autotoc_md58", null ],
        [ "Folly", "md_doc_design_ru.html#autotoc_md59", null ],
        [ "HPX", "md_doc_design_ru.html#autotoc_md60", null ],
        [ "Boost.Fiber и marl", "md_doc_design_ru.html#autotoc_md61", null ],
        [ "Taskflow", "md_doc_design_ru.html#autotoc_md62", null ],
        [ "libunifex/cppcoro", "md_doc_design_ru.html#autotoc_md63", null ]
      ] ]
    ] ],
    [ "Install", "md_doc_install.html", [
      [ "Prerequisites", "md_doc_install.html#autotoc_md65", [
        [ "Linux", "md_doc_install.html#autotoc_md66", null ],
        [ "macOS", "md_doc_install.html#autotoc_md67", null ],
        [ "Windows", "md_doc_install.html#autotoc_md68", null ]
      ] ],
      [ "As library", "md_doc_install.html#autotoc_md69", [
        [ "CMake with FetchContent", "md_doc_install.html#autotoc_md70", null ]
      ] ],
      [ "As project for develop", "md_doc_install.html#autotoc_md71", [
        [ "Console install and CMake/build options", "md_doc_install.html#autotoc_md72", [
          [ "Example", "md_doc_install.html#autotoc_md73", null ]
        ] ]
      ] ]
    ] ],
    [ "pr_guide", "md_doc_pr_guide.html", null ],
    [ "Sanitizers", "md_doc_sanitizer.html", [
      [ "Address Sanitizer", "md_doc_sanitizer.html#autotoc_md79", [
        [ "Tips", "md_doc_sanitizer.html#autotoc_md75", null ],
        [ "Useful links", "md_doc_sanitizer.html#autotoc_md76", null ],
        [ "Options", "md_doc_sanitizer.html#autotoc_md77", [
          [ "Useful options", "md_doc_sanitizer.html#autotoc_md78", null ]
        ] ],
        [ "Useful links", "md_doc_sanitizer.html#autotoc_md80", null ],
        [ "Overhead", "md_doc_sanitizer.html#autotoc_md81", null ],
        [ "ASAN_OPTIONS", "md_doc_sanitizer.html#autotoc_md82", [
          [ "Useful options", "md_doc_sanitizer.html#autotoc_md83", null ]
        ] ]
      ] ],
      [ "Thread Sanitizer", "md_doc_sanitizer.html#autotoc_md84", [
        [ "Useful links", "md_doc_sanitizer.html#autotoc_md85", null ],
        [ "Overhead", "md_doc_sanitizer.html#autotoc_md86", null ],
        [ "TSAN_OPTIONS", "md_doc_sanitizer.html#autotoc_md87", [
          [ "Useful options", "md_doc_sanitizer.html#autotoc_md88", null ]
        ] ],
        [ "DSAN_OPTIONS", "md_doc_sanitizer.html#autotoc_md89", null ]
      ] ],
      [ "Undefined Behavior Sanitizer", "md_doc_sanitizer.html#autotoc_md90", null ],
      [ "Memory Sanitizer", "md_doc_sanitizer.html#autotoc_md91", null ]
    ] ],
    [ "Style guide", "md_doc_style_guide.html", [
      [ "Formatting", "md_doc_style_guide.html#autotoc_md93", null ],
      [ "Code style", "md_doc_style_guide.html#autotoc_md94", null ]
    ] ],
    [ "Targets", "md_doc_target.html", [
      [ "yaclib", "md_doc_target.html#autotoc_md96", [
        [ "CLion", "md_doc_target.html#autotoc_md97", null ]
      ] ]
    ] ],
    [ "Todo List", "todo.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"dir_9039413c522f41ada5cc655e104f9ea8.html",
"namespaceyaclib_1_1util.html#a21c5f67e7ba7f9dac6c4c08e8c8409ad"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';