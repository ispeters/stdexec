import json
import collections

d = json.load(open("entity_graph.json"))

# all files mentioned as a "from" where one side of the edge is in stdexec
all_sources = set()
# the mappings from target to the list of sources that include it
backwards = collections.defaultdict(lambda: [])

for e in d["edges"]:
    source = e["from"]
    target = e["to"]
    if "stdexec/" in target and "def" in e["strength"]:
        all_sources.add(source)
        backwards[target].append(source)

# all files mentioned as a "to" where one side of the edge is in stdexec
all_targets = set(backwards.keys())

# a list of lists
#  - levels[0] is all the files that are mentioned as a "to" but not mentioned
#    as a "from"
#  - levels[n] is all the files that include any file in levels[n - 1]
levels = []

while len(all_targets) > 0:
    level = []
    # add to the current level all targets that are not sources
    for target in all_targets:
        if not target in all_sources:
            level.append(target)

    # for each target in level, remove from all_sources all the
    # files that include target directly, which makes the affected
    # sources "targets that are not sources" in the next iteration
    for target in level:
        for source in backwards[target]:
            if source in all_sources:
                all_sources.remove(source)

        # also remove target from all_targets so it's never added
        # to another level
        all_targets.remove(target)

    print("level " + str(len(levels)) + ": " + str(level))

    levels.append(level)

for idx, level in enumerate(levels, 0):
    file = open(f"modules/stdexec.level_{idx}.cppm", "w")
    print("module;", file=file)
    print("", file=file)
    print("#include <cassert>", file=file)
    print("#include <cstdarg>", file=file)
    print("#include <cstdint>", file=file)
    print("#include <cstdio>", file=file)
    print("#include <cstdlib>", file=file)
    print("", file=file)
    print(f"export module stdexec.level_{idx};", file=file)
    print("", file=file)
    j = 0
    while j < idx:
        print(f"import stdexec.level_{j};", file=file)
        j = j + 1

    if idx > 0:
        print("", file=file)

    print("#pragma clang diagnostic push", file=file)
    print("#pragma clang diagnostic ignored \"-Winclude-angled-in-module-purview\"", file=file)
    print("", file=file)
    print("#include <stdexec/__detail/__config.hpp>", file=file)
    print("", file=file)

    print("#define STDEXEC_IN_MODULE_PURVIEW", file=file)
    print("", file=file)

    for include in sorted(level):
        print(f"#include <{include}>", file=file)

    print("", file=file)
    print("#pragma clang diagnostic pop", file=file)
