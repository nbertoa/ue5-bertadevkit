# AGENTS.md

## Project Contract

This repository contains **BertaDevKit**, a personal Unreal Engine 5.8 toolkit for R&D, prototyping, debugging, editor
automation, validation, and everyday development.

BertaDevKit exists to turn recurring development friction into small, reusable, maintainable utilities.

### Engine Target

BertaDevKit is **Unreal Engine 5.8 only**.

Do not add backward compatibility, engine-version macros, legacy workarounds, or support for older Unreal versions
unless explicitly requested.

For version-sensitive Unreal behavior, verify against UE 5.8 documentation, engine source, or a reproducible experiment.
Never invent Unreal APIs or behavior.

### Repository Structure

The repository contains a host project around the plugin:

```text
ue5-bertadevkit/
├── AGENTS.md
├── README.md
└── BertaDevKitHost/
    ├── BertaDevKitHost.uproject
    ├── Content/
    └── Plugins/
        └── BertaDevKit/
            ├── BertaDevKit.uplugin
            └── Source/
                ├── BertaDevKit/
                └── BertaDevKitEditor/
```

The actual plugin root is:

```text
BertaDevKitHost/Plugins/BertaDevKit/
```

The host project exists for development and verification.

### Scope

A feature belongs in BertaDevKit when it addresses a concrete recurring need and is reusable across projects, such as:

* debugging and observability;
* visualization;
* Blueprint or C++ helpers;
* Editor automation;
* validation;
* prototyping support;
* reduction of repeated setup or boilerplate.

Do not add functionality merely because it could be useful someday.

Do not recreate a solid Unreal Engine feature without a concrete usability benefit from wrapping or extending it.

Do not replace specialized plugins without a demonstrated gap.

Keep a feature inside BertaDevKit while it remains a cohesive utility. Consider extracting it only when it becomes an
independent product or system with substantial API, dependencies, configuration, or lifecycle of its own.

### Module Boundaries

Maintain strict separation:

* `BertaDevKit`: Runtime.
* `BertaDevKitEditor`: Editor-only.

Runtime must never depend on Editor modules.

Keep Runtime dependencies minimal.

A dependency belongs in `PublicDependencyModuleNames` only when it is required by the module's public API. Prefer
private dependencies for implementation details.

Optional integrations should avoid hard dependencies when practical.

Do not add modules or plugins preemptively.

## Language

Communicate with the user in Spanish by default.

Keep the following in English:

* source code;
* identifiers;
* Unreal Engine APIs;
* filenames;
* module names;
* compiler and linker errors;
* logs;
* commit messages.

Do not translate technical identifiers unnecessarily.

## Source of Truth

Treat the current working tree as the operational source of truth.

Before making a significant change:

* inspect the relevant implementation;
* inspect nearby tests and callers;
* read all applicable `AGENTS.md` files;
* check existing project conventions.

Do not assume README documentation, comments, old audits, or previous discussions are more current than the code.

Do not fetch, reset, rebase, checkout another branch, discard changes, or otherwise alter repository state unless
explicitly requested.

If existing user changes affect the task, preserve them and work around them.

## Collaboration Workflow

Use this default cycle:

```text
Understand
→ Constrain
→ Design
→ Implement
→ Verify
→ Review
→ Integrate
→ Learn
```

Discussion does not authorize modification.

Authorization to modify does not authorize commit, push, branch creation, or pull requests.

Before implementation, understand:

* the real problem;
* expected behavior;
* scope;
* ownership;
* lifecycle;
* contracts;
* invariants;
* failure paths;
* architectural impact;
* verification strategy.

Work on one coherent unit at a time.

Do not silently expand scope.

Do not perform unrelated cleanup, refactoring, formatting, renaming, dependency changes, or documentation work. Record
additional work separately instead.

Prefer the smallest change that fully solves the problem.

## Engineering Priorities

Default order:

1. Correctness.
2. Clear responsibility and ownership.
3. Simplicity.
4. Maintainability.
5. Observability and diagnosability.
6. Extensibility when justified.
7. Performance when supported by evidence.
8. Style and polish.

Simplicity means minimizing concepts, coupling, dependencies, hidden state, and difficult lifecycle interactions — not
merely minimizing lines of code.

## Design Principles

Prefer:

* cohesive responsibilities;
* explicit ownership and lifecycle;
* small APIs;
* clear data flow;
* composition over inheritance when appropriate;
* limited and intentional state;
* concrete solutions over speculative abstractions.

Avoid:

* generic managers without a concrete responsibility;
* hidden global state;
* speculative extension points;
* settings without demonstrated need;
* abstraction created only to remove small amounts of duplication.

Accept limited duplication when the alternative is a premature abstraction.

## Unreal Engine Guidance

Always consider, where relevant:

* UObject ownership and Garbage Collection;
* Actor and Component lifecycle;
* reflection;
* `UPROPERTY` and `UFUNCTION`;
* strong, weak, and non-owning references;
* `TObjectPtr` for reflected UObject ownership where appropriate;
* delegates and symmetric unregistration;
* timers and lifetime;
* async work and game-thread requirements;
* world contexts;
* Editor versus PIE versus Standalone versus packaged behavior;
* replication and authority;
* asset references;
* module loading and shutdown;
* build configuration differences.

Use C++ for stable contracts, ownership, architecture, complex logic, and performance-sensitive implementation.

Use Blueprint for composition, configuration, experimentation, and iteration.

Neither should be chosen dogmatically.

### Blueprint APIs

Blueprint exposure must be deliberate.

Prefer:

* clear names;
* consistent categories;
* safe defaults;
* few visible pins;
* `AdvancedDisplay` for secondary options;
* explicit failure behavior;
* appropriate pure/callable semantics.

Do not expose implementation details merely because they can be exposed.

## Defensive Programming

Classify conditions before choosing a mechanism.

Distinguish:

* preconditions;
* postconditions;
* invariants;
* expected runtime failures;
* programming defects;
* recoverable failures;
* non-recoverable failures.

Prefer preventing invalid states through ownership and architecture.

Do not compensate for unclear ownership with blanket null checks.

Use:

* `check` / `checkf` for invariants and programming defects when continuing is unsafe;
* `ensure` / `ensureMsgf` when a defect should be diagnosed but safe continuation is possible;
* normal validation and explicit failure for expected external or runtime conditions;
* early returns when they make the valid execution path clearer.

Never silently continue from an unsafe or partially invalid state.

Logs and assertions should contain useful diagnostic context.

## Logging and Observability

Use project-specific log categories.

Choose verbosity according to meaning.

Do not leave noisy logs in hot paths without justification.

Failures should be observable enough to diagnose without requiring guesswork.

## Performance

Do not optimize based on intuition alone.

Consider performance when:

* the code runs frequently;
* it iterates large collections;
* it allocates in hot paths;
* it loads assets;
* it performs expensive world queries;
* profiling or measurements show a problem.

Prefer evidence before increasing complexity.

## Includes, IWYU, and Dependencies

Follow Unreal's IWYU expectations.

A `.cpp` should directly include what it uses instead of relying on transitive includes.

Avoid unnecessary module dependencies.

When changing `Build.cs`, understand why each dependency is required and whether it belongs in Public or Private
dependencies.

Do not add a dependency solely to silence a compile error without understanding the ownership/API relationship.

## Documentation

Do not turn the repository into an encyclopedia.

Document:

* durable decisions;
* non-obvious contracts;
* important setup;
* lifecycle requirements;
* behavior that would otherwise be easy to misuse.

Prefer updating an existing source over creating another document.

Comments should explain intent, constraints, or non-obvious behavior rather than restating code.

Keep README-level documentation durable. Avoid duplicating counts, private implementation details, or facts likely to
change frequently.

## Unreal Editor Execution Policy

Do not launch any Unreal Editor process unless Nicolás explicitly authorizes it for the current task.

This includes:

* `UnrealEditor.exe`;
* `UnrealEditor-Cmd.exe`;
* commandlets launched through an Unreal Editor executable;
* PIE or Standalone sessions;
* Unreal Automation Tests that require starting Unreal;
* interactive Editor, Content Browser, Slate, UI, or `DebugMap` smoke tests.

By default, Codex may:

* inspect and modify source files;
* inspect Unreal Engine source;
* run Git commands authorized by the task;
* perform static analysis;
* compile through UBT / `Build.bat`;
* review compiler warnings, IWYU, dependencies, and the final diff;
* add or update Automation Tests without executing them.

Generic instructions such as `run tests`, `run Automation Tests`, `verify`, `smoke test`, or `build/test` do not
authorize launching Unreal.

Authorization to modify, build, test, commit, push, or integrate does not authorize launching Unreal.

Only an explicit instruction from Nicolás equivalent to `launch Unreal and test it` authorizes starting an Unreal
process.

When behavioral verification requires Unreal:

1. do not launch Unreal automatically;
2. complete all available static and compile-time verification;
3. mark the Unreal verification as pending;
4. provide Nicolás with exact manual test steps.
5. do not claim the behavior was verified until evidence is available.

## Verification

Match verification effort to risk.

Never claim a check was executed unless it actually was.

### Documentation-only changes

Normally verify:

* Markdown/content correctness;
* repository-relative paths and links;
* final diff.

Do not run a full Unreal build solely because documentation changed unless the task specifically requires validating
documented commands or behavior.

### C++ or build changes

Baseline when relevant:

1. Build `BertaDevKitHostEditor Win64 Development` with UE 5.8 through UBT.
2. Add or update affected Unreal Automation Tests when appropriate, but do not launch Unreal to execute them unless
   Nicolás explicitly authorizes it.
3. Review new warnings.
4. Review headers, IWYU, and module dependencies.
5. Inspect the complete final diff.

### Blueprint, Editor tooling, or visual behavior

Do not automatically launch Unreal Editor for smoke testing. When Editor, Blueprint, Content Browser, Slate, PIE, or
visual verification is required:

1. provide Nicolás with exact manual verification steps;
2. mark that behavioral verification as pending until Nicolás reports the result.

`/Game/Maps/DebugMap` may be used by Nicolás for manual smoke testing:

```text
/Game/Maps/DebugMap
```

### Runtime or packaging-sensitive changes

When the change can affect packaged behavior or Runtime compatibility, perform the appropriate build/package
verification when feasible.

If a required verification cannot be run in the current environment, state exactly what remains unverified and provide
the command or steps needed.

Always distinguish:

* verified;
* inferred;
* not verified.

## Code Review

Review in this order:

1. Correctness.
2. Crashes, corruption, and undefined behavior.
3. Ownership and lifecycle.
4. Invariants and failure handling.
5. Architecture and responsibilities.
6. API design and coupling.
7. Concurrency, replication, and performance.
8. Maintainability and readability.
9. Style.

Distinguish defects from preferences.

Every finding should explain its concrete consequence.

Review the full diff, including error paths and partial states, before considering the work finished.

## Git Policy

Use this normal implementation flow:

```text
discuss
→ investigate
→ modify
→ build
→ review
→ commit
→ push
```

For an explicitly authorized implementation task, Codex should by default commit and push directly to `main` when the
work is complete, provided that:

* the permitted build verification passed;
* the complete diff review found no defects that make integration unsafe;
* no unrelated user changes are included;
* repository state allows a clean integration.

Do not create branches or pull requests unless Nicolás explicitly requests them.

A discussion, investigation, diagnosis, or review does not authorize modification, commit, or push.

Do not reset, rebase, discard, overwrite, or otherwise destroy Nicolás's work.

Pending manual Unreal verification does not automatically prevent commit/push. Complete all verification allowed by the
Unreal execution policy, report the manual verification as pending, and judge integration according to the actual risk
of the change.

After an integrated implementation:

* summarize the final diff;
* report build and verification performed;
* report pending manual verification;
* report the Conventional Commit used;
* report the commit SHA;
* confirm whether the push to `main` succeeded.

## Confidentiality and Portability

BertaDevKit must remain reusable and independent from confidential external projects.

Do not introduce proprietary project code, names, assets, credentials, paths, or assumptions.

Prefer utilities that can be moved between Unreal projects without unnecessary project-specific dependencies.

## Final Standard

Understand before implementing.

Make ownership, lifecycle, contracts, invariants, and failure behavior explicit.

Choose the simplest design that is correct, maintainable, and verifiable.

Change only what the task requires.

Verify according to risk.

Review the complete result before integration.
