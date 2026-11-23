# Project subagent workflow

Integrated from `E:\Users\Downloads\SubAgents` on 2026-11-22, with project-specific context and an
additional repository maintenance role. The original Downloads files remain the source kit.

## Roles

| Role | Responsibility | Default effort |
| --- | --- | --- |
| `planner` | Feature options, product trade-offs, phased architecture plans | high |
| `data_architect` | Stored identity, relationships, schema, compatibility, migration | high |
| `project_maintainer` | File placement, CMake, agent setup, Git workflow advice | high |
| `reviewer` | Independent findings about a concrete diff | high |
| `tester` | Reproduction and non-destructive behavioral verification | medium |

The main agent applies changes and reconciles results. All roles inherit the parent's model; effort is
set per role. Advisory roles do not edit. Tester can generate isolated verification artifacts, subject
to its instructions. Simple changes do not require a specialist.

`.codex/config.toml` enables agents and caps concurrently open spawned threads at three, excluding the
main thread. Standalone role files are discovered under `.codex/agents/`; their `name` fields identify
the roles. These settings follow the [official subagent documentation](https://learn.chatgpt.com/docs/agent-configuration/subagents).
The session or managed environment can impose a lower limit. Role instructions do not establish separate
read-only sandboxes; permissions remain inherited from the parent.

## Starting a session

Open this repository in a fresh Codex session to pick up the project configuration and merged AGENTS.md.
If the running client has not refreshed project settings, restart it. Project-local configuration is
subject to client trust and managed policy. Do not relax permissions just to enable a role.

Try a bounded request:

```text
Use project_maintainer to inspect our source layout and CMake targets. Recommend where new
protocol helpers should go and identify the references that a move would affect. Research only;
do not edit files or Git state. Summarize the recommendation in the main thread.
```

```text
Use planner to explore configurable weighted ranking. Research only. Consult data_architect
only if persisted data or query contracts would change. Separate existing behavior from proposals.
```

```text
Have reviewer inspect this task's diff for actionable regressions. Have tester verify the
affected acceptance criteria using isolated test data. Summarize actual results and gaps.
```

Before concurrent checks, inspect scripts and separate any mutable build or test artifacts. Never point
verification at personal `.dat` files. If roles are unavailable in a session, the main agent can inspect
their instructions and do the work itself, but must not report that a custom agent was loaded or run.

## Maintaining context

Start with the task, AGENTS.md and the affected source/tests. Each role names relevant paths in the
actual C++ backend, CMake and supporting scripts. Load documentation only when it helps the question:
request-response-map.md for flow context, library-wire-format.md for protocol intent, and
storyline-groups.md for grouping/rank policy. Verify those documents against current code.

The current-state, architecture, data-contract, vision and roadmap files added during integration are
optional summaries, not required inputs or a second source of requirements. AGENTS.md defines the active
Alpha milestone. The kit's UI ideas remain unapproved possibilities. Existing target architecture docs
remain proposals unless code and an approved decision establish otherwise. Use plan/decision templates
for substantial work; ordinary file or build fixes do not need new approval gates.
