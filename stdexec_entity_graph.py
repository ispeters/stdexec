#!/usr/bin/env python3
"""
Entity-level dependency graph for stdexec.

The header include graph is not enough: __execution_fwd.hpp forward-declares ~114
entities, which launders real cycles into an apparent DAG. This walks the Clang AST
instead and records, per entity, WHERE IT IS DECLARED and WHERE IT IS DEFINED, then
emits edges annotated by strength:

    needs-decl  -- a mention that a forward declaration satisfies
    needs-def   -- a mention that requires the definition (complete type, base class,
                   member access, template instantiation, concept use, CPO object use)

Only `needs-def` edges constrain the module DAG. `needs-decl` edges can be satisfied
by a shared `:fwd` partition.

Usage:
    # from `devshell stdexec`, with a configured build dir
    cmake --preset llvm-debug-modules -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    python3 stdexec_entity_graph.py build/compile_commands.json --root include

Requires: python3Packages.libclang (or clang.cindex from the LLVM install), networkx.
"""

import argparse, json, os, re, shlex, subprocess, sys, collections
import clang.cindex as ci


def build_argv(cmd):
    """
    Extract compiler args for libclang from a compile_commands.json entry.

    Naive `cmd["command"].split()` filtering of ("-c", "-o") tokens leaves
    -o's *value* (the output object path) behind as a stray positional
    argument, which then collides with the filename passed separately to
    index.parse() and makes libclang refuse to construct a command line at
    all -- that's the root cause of the zero-detail "Error parsing
    translation unit" on every file, even trivial ones. Also use shlex
    instead of str.split() for shell-correct tokenization.
    """
    toks = shlex.split(cmd["command"])[1:]  # drop the compiler executable
    src = os.path.abspath(cmd["file"])
    out, skip_next = [], False
    for a in toks:
        if skip_next:
            skip_next = False
            continue
        if a in ("-o", "-MF", "-MT", "-MQ"):
            skip_next = True
            continue
        if a in ("-c",):
            continue
        if os.path.isabs(a) and os.path.abspath(a) == src:
            continue  # the source file itself; passed separately to parse()
        out.append(a)
    return out


def wrapped_compiler_isystem_dirs(cxx=None):
    """
    Ask the wrapped compiler (via `clang++ -E -v -`) for its real, final
    include search path, rather than trying to reconstruct it from
    NIX_CFLAGS_COMPILE / NIX_CXXSTDLIB_COMPILE. On this toolchain neither
    env var alone captures it -- some of it (e.g. libcxx's include/c++/v1,
    the Darwin SDK's usr/include, and its Frameworks dir) is baked into the
    wrapper at build time and never appears in the environment at all.
    Framework directories are dropped since libclang's -isystem doesn't
    want them; pass a real -F if a TU ever needs one.
    """
    cxx = cxx or os.environ.get("CXX", "clang++")
    out = subprocess.run(
        [cxx, "-E", "-x", "c++", "-v", "-"],
        input="", capture_output=True, text=True,
    ).stderr
    m = re.search(
        r"#include <\.\.\.> search starts here:\n(.*?)\nEnd of search list\.",
        out, re.S,
    )
    if not m:
        print("warning: couldn't parse compiler -v output for include dirs",
              file=sys.stderr)
        return []
    dirs = []
    for line in m.group(1).splitlines():
        line = line.strip()
        if line.endswith("(framework directory)"):
            continue
        if line:
            dirs.append(line)
    return [f"-isystem{d}" for d in dirs]

# Cursor kinds whose *use* requires a complete definition, not just a declaration.
NEEDS_DEF_KINDS = {
    ci.CursorKind.CXX_BASE_SPECIFIER,
    ci.CursorKind.MEMBER_REF_EXPR,
    ci.CursorKind.MEMBER_REF,
    ci.CursorKind.CALL_EXPR,
    ci.CursorKind.CONCEPT_SPECIALIZATION_EXPR
        if hasattr(ci.CursorKind, "CONCEPT_SPECIALIZATION_EXPR") else ci.CursorKind.UNEXPOSED_EXPR,
}

REF_KINDS = {
    ci.CursorKind.TYPE_REF,
    ci.CursorKind.TEMPLATE_REF,
    ci.CursorKind.DECL_REF_EXPR,
    ci.CursorKind.MEMBER_REF_EXPR,
    ci.CursorKind.MEMBER_REF,
    ci.CursorKind.CXX_BASE_SPECIFIER,
    ci.CursorKind.OVERLOADED_DECL_REF,
    ci.CursorKind.CALL_EXPR,  # CPO invocations (connect, schedule, set_value, ...)
    # NAMESPACE_REF deliberately excluded: namespaces have no single
    # definition, so get_definition() on one arbitrarily returns whichever
    # reopening fragment it happens to land on, producing edges to files
    # that merely reopen `namespace std`/`namespace STDEXEC` and have no
    # real relationship to the referencing code.
}


def rel(path, root):
    if not path:
        return None
    p = os.path.abspath(path)
    r = os.path.abspath(root)
    return os.path.relpath(p, r) if p.startswith(r + os.sep) else None


class Index:
    """Maps USR -> {decl_files, def_files, kind, spelling}."""

    def __init__(self, root):
        self.root = root
        self.ents = collections.defaultdict(
            lambda: {"decl": set(), "def": set(), "kind": None, "name": None}
        )
        # (from_file, to_file, strength) -> count
        self.edges = collections.Counter()

    def note_decl(self, c):
        usr = c.get_usr()
        if not usr:
            return
        f = rel(c.location.file.name if c.location.file else None, self.root)
        if not f:
            return
        e = self.ents[usr]
        e["kind"] = e["kind"] or str(c.kind)
        e["name"] = e["name"] or c.spelling
        (e["def"] if c.is_definition() else e["decl"]).add(f)

    def note_ref(self, c, enclosing_file):
        ref = c.referenced
        if ref is None:
            return
        usr = ref.get_usr()
        if not usr:
            return
        # Where does the thing this mention needs actually live?
        d = ref.get_definition()
        strength = "needs-def" if c.kind in NEEDS_DEF_KINDS else "needs-decl"
        target = None
        if d is not None and d.location.file:
            target = rel(d.location.file.name, self.root)
        if target is None and ref.location.file:
            target = rel(ref.location.file.name, self.root)
            strength = "needs-decl"  # only a declaration was reachable
        if target and target != enclosing_file:
            self.edges[(enclosing_file, target, strength)] += 1


def walk(cursor, idx, seen):
    stack = [cursor]
    while stack:
        c = stack.pop()
        loc = c.location.file.name if c.location.file else None
        f = rel(loc, idx.root)
        if f:
            if c.is_definition() or c.kind.is_declaration():
                idx.note_decl(c)
            if c.kind in REF_KINDS:
                idx.note_ref(c, f)
        for ch in c.get_children():
            key = (ch.hash, ch.location.offset)
            if key in seen:
                continue
            seen.add(key)
            stack.append(ch)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("compile_commands")
    ap.add_argument("--root", default="include", help="include root to keep edges within")
    ap.add_argument("--libclang", help="path to libclang.so/.dylib")
    ap.add_argument("--out", default="entity_graph.json")
    ap.add_argument("--limit", type=int, default=0, help="only parse first N TUs")
    args = ap.parse_args()

    if args.libclang:
        ci.Config.set_library_file(args.libclang)

    cmds = json.load(open(args.compile_commands))
    src_root = os.path.abspath(args.root)
    before = len(cmds)
    cmds = [
        c for c in cmds
        if os.path.abspath(c["file"]).startswith(os.path.dirname(src_root) + os.sep)
        and "@" not in c["command"]  # drop response-file (module-map) invocations
    ]
    print(f"filtered {before} -> {len(cmds)} compile-commands entries "
          f"(dropped toolchain/module-map entries outside the project)",
          file=sys.stderr)
    if args.limit:
        cmds = cmds[: args.limit]

    sysroot_extra = wrapped_compiler_isystem_dirs()
    print(f"resolved {len(sysroot_extra)} system include dirs from "
          f"$CXX -v", file=sys.stderr)

    idx = Index(args.root)
    index = ci.Index.create()
    for i, cmd in enumerate(cmds, 1):
        argv = build_argv(cmd) + sysroot_extra
        src = cmd["file"]
        print(f"[{i}/{len(cmds)}] {os.path.basename(src)}", file=sys.stderr)
        try:
            tu = index.parse(src, args=argv,
                             options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        except ci.TranslationUnitLoadError as e:
            print(f"  !! {e}", file=sys.stderr)
            if i <= 2:  # show the actual argv on the first couple failures
                print(f"     argv: {argv}", file=sys.stderr)
            continue
        fatal = [d for d in tu.diagnostics if d.severity >= d.Error]
        if fatal:
            print(f"  !! {len(fatal)} error diagnostic(s):", file=sys.stderr)
            for d in fatal[:5]:
                print(f"     {d}", file=sys.stderr)
        walk(tu.cursor, idx, set())

    out = {
        "entities": {
            u: {"name": e["name"], "kind": e["kind"],
                "decl": sorted(e["decl"]), "def": sorted(e["def"])}
            for u, e in idx.ents.items()
            if e["decl"] or e["def"]
        },
        "edges": [
            {"from": a, "to": b, "strength": s, "count": n}
            for (a, b, s), n in idx.edges.items()
        ],
    }
    json.dump(out, open(args.out, "w"), indent=1)
    print(f"\n{len(out['entities'])} entities, {len(out['edges'])} edges -> {args.out}",
          file=sys.stderr)

    strength_counts = collections.Counter(e["strength"] for e in out["edges"])
    print(f"edge strengths: {dict(strength_counts)}", file=sys.stderr)

    # --- immediate payoff: SCCs over needs-def edges only ---
    try:
        import networkx as nx
    except ImportError:
        return
    G = nx.DiGraph()
    for e in out["edges"]:
        if e["strength"] == "needs-def":
            G.add_edge(e["from"], e["to"], weight=e["count"])
    sccs = [c for c in nx.strongly_connected_components(G) if len(c) > 1]
    print(f"\n{len(sccs)} nontrivial SCCs over needs-def edges:", file=sys.stderr)
    for c in sorted(sccs, key=len, reverse=True):
        print(f"  --- {len(c)} files ---", file=sys.stderr)
        for f in sorted(c):
            print(f"      {f}", file=sys.stderr)
    cond = nx.condensation(G, scc=list(nx.strongly_connected_components(G)))
    print(f"\ncondensed DAG: {cond.number_of_nodes()} nodes, "
          f"{cond.number_of_edges()} edges", file=sys.stderr)
    # candidate root nodes = SCCs with no outgoing needs-def edges
    roots = [n for n in cond if cond.out_degree(n) == 0]
    print(f"root (dependency-free) components: {len(roots)}", file=sys.stderr)
    for n in roots:
        print("   ", sorted(cond.nodes[n]["members"]), file=sys.stderr)


if __name__ == "__main__":
    main()
