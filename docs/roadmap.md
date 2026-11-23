# Roadmap status

AGENTS.md contains the milestone definitions and acceptance criteria. This file is a navigation summary,
not an independent set of approvals. The agent kit does not change milestone scope or claim completion.

## Active: Alpha

Deliver and demonstrate authenticated client workflows for groups and child entries, stable identity,
permissions and ownership consistency, membership, compilation, safe failure, and successful save/restart.
Use isolated files for automated acceptance checks and demonstrate the same flow through a real client.
Inspect current-state.md and code to identify gaps; do not assume a listed method meets acceptance.

## Later milestones

- Beta: extract proven common database logic while preserving entity-specific rules.
- Delta: improve consistent errors and diagnostics beyond Alpha's basic failure reporting.
- Epsilon: define a workload and target, then implement and measure bounded network concurrency.
- Zeta: profile representative workloads before optimizing, including possible SIMD work.

Later milestones are not prerequisites for Alpha. Deferred work remains deferred as specified in AGENTS.md.

## Unapproved opportunities from the kit

Configurable weighted ranking, card expansion, richer navigation and personalized views need a scoped
product request before planning implementation. Use planner for consequential behavior and data_architect
when storage/contracts change. Use project_maintainer for supporting file/build/workflow organization.
