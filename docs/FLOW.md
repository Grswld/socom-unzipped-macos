# Flow: how work moves through this repository

> **Generated -- do not edit.** Written by `python -m tools_py.flow --since 2026-09-01` (Sprint 14 M1, the review's F8; tokens M2) from the git history at the commit named below, the lock's queue log, `docs/BACKLOG.md` and, when SOCOM_CLAUDE_TRANSCRIPTS names a directory, the sums of its transcripts' usage fields. The methods are note 03's (`docs/audits/2026-09-26-autonomy-structure-review/03-git-forensics.md`); each number names the command that reproduces it, and where the method differs from the note's the module docstring says how. Descriptive, not targets. `python -m tools_py.flow --check` exits 1 when this file is not a render of the commit it names; regenerate at the close.

Rendered at commit `7d12f7eddc06a1e46d2dcaf2ebfc8ef2a3cf2928` (committer time 2026-10-12T09:37:40Z), since 2026-09-01 (UTC days): 3375 commits, 465 merges.

## The numbers

- **merges per day**: 465 merges on 20 days (mean 23.2 a merge day); `git log --merges` counts 465. Per day below -- command: `python -m tools_py.changelog --rev 7d12f7ed` (its merges, changelog.entries()), by UTC committer day; cross-check: `TZ=UTC git log --merges --since=2026-09-01T00:00:00Z --date=format-local:%F --format=%cd 7d12f7ed | sort | uniq -c`
- **fix rounds per merge**: rounds: merges -- 0: 409 (88.0 %); 1: 41 (8.8 %); 2: 9 (1.9 %); 3: 4 (0.9 %); 4: 1 (0.2 %); 7: 1 (0.2 %); none: 88.0 %, at most one: 96.8 %. A round counts once, at the first merge whose range brings it in; merge commits and docs(sprint-N) plan commits are not rounds; rounds committed straight on a sprint branch count at the sprint's merge into main (Sprint 11's seven at PR #49) -- command: `git log --no-merges --format=%s P1..P2 | grep -v -E '^docs\(sprint-[0-9]+\)' | grep -c -E 'review round|fix round'` per merge with parents P1 P2, oldest first, less the commits an earlier merge counted (note 03 section 2.2, deduplicated)
- **fix rounds per merge, broad pattern**: matches: merges -- 0: 227 (48.8 %); 1: 135 (29.0 %); 2: 61 (13.1 %); 3: 14 (3.0 %); 4: 10 (2.2 %); 5: 4 (0.9 %); 6: 1 (0.2 %); 7: 3 (0.6 %); 8: 2 (0.4 %); 9: 1 (0.2 %); 10: 2 (0.4 %); 16: 1 (0.2 %); 19: 1 (0.2 %); 28: 1 (0.2 %); 31: 1 (0.2 %); 32: 1 (0.2 %); none: 48.8 %, at most one: 77.8 % (an upper bound: every fix(scope) commit counts). The same rule -- command: `git log --no-merges --format=%s P1..P2 | grep -v -E '^docs\(sprint-[0-9]+\)' | grep -c -E 'review round|fix round|fix\(|review'` per merge, oldest first, less the commits an earlier merge counted
- **subjects saying " again"**: 59 of 3375 commits -- command: `git log --since=2026-09-01T00:00:00Z --format=%s 7d12f7ed | grep -ci -- ' again'`
- **docs share of churn, seven days**: 64.3 %: 10392 of 16167 lines added + deleted, 2026-10-05T09:37:40Z to 2026-10-12T09:37:40Z -- command: `git log --numstat --no-renames --since=2026-10-05T09:37:40Z --until=2026-10-12T09:37:40Z --format= 7d12f7ed | awk '$1 ~ /^[0-9]+$/ {t += $1 + $2} $3 ~ /^docs\// {d += $1 + $2} END {print d, t}'`
- **ticket wait**: median 1363 s over 65 tickets -- command: `grep -E 'TICKET [^ ]+ waited [0-9]+' logs/loop_lock.queue.log` (the main tree's), the median of the last field, unstamped lines and lines stamped after 2026-10-12T09:37:40Z or before 2026-09-01 left out
- **open-issue age**: not measured: the open-issues table of docs/BACKLOG.md at `7d12f7ed` has no Opened column; the dates live on GitHub (`gh issue list --state open --json number,createdAt`), a network read this page does not make -- command: `git show 7d12f7ed:docs/BACKLOG.md`, the Opened column of section 1's table, days to 2026-10-12
- **sessions**: 48 distinct Claude-Session trailers -- command: `git log --since=2026-09-01T00:00:00Z --format=%B 7d12f7ed | grep '^Claude-Session:' | sort -u | wc -l`
- **commits per session**: largest first: 333, 255, 237, 220, 85, 69, 66, 61, 32, 30, 24, 22, 22, 21, 19, 17, 16, 12, 10, 9, 8, 7, 7, 7, 6, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1; median 6.5 -- command: `git log --since=2026-09-01T00:00:00Z --format=%B 7d12f7ed | grep '^Claude-Session:' | sort | uniq -c | sort -rn`
- **token spend**: not measured: SOCOM_CLAUDE_TRANSCRIPTS is not set. The transcripts are local to the machine that ran the sessions, so the page sums them only when the controller names their directory -- command: `SOCOM_CLAUDE_TRANSCRIPTS=<the transcripts directory> python -m tools_py.flow --usage` (Claude Code's are under ~/.claude/projects/<project-key>/), the `message.usage` fields of each `.jsonl` summed

## Merges per UTC day

| day | merges |
|---|---:|
| 2026-09-08 | 1 |
| 2026-09-18 | 2 |
| 2026-09-20 | 1 |
| 2026-09-21 | 25 |
| 2026-09-22 | 7 |
| 2026-09-23 | 18 |
| 2026-09-24 | 7 |
| 2026-09-25 | 47 |
| 2026-09-26 | 65 |
| 2026-09-27 | 48 |
| 2026-09-28 | 62 |
| 2026-09-29 | 115 |
| 2026-09-30 | 18 |
| 2026-10-01 | 9 |
| 2026-10-02 | 10 |
| 2026-10-03 | 15 |
| 2026-10-04 | 3 |
| 2026-10-09 | 4 |
| 2026-10-11 | 6 |
| 2026-10-12 | 2 |
