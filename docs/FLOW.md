# Flow: how work moves through this repository

> **Generated -- do not edit.** Written by `python -m tools_py.flow --since 2026-09-01` (Sprint 14 M1, the review's F8; tokens M2) from the git history at the commit named below, the lock's queue log, `docs/BACKLOG.md` and, when SOCOM_CLAUDE_TRANSCRIPTS names a directory, the sums of its transcripts' usage fields. The methods are note 03's (`docs/audits/2026-09-26-autonomy-structure-review/03-git-forensics.md`); each number names the command that reproduces it, and where the method differs from the note's the module docstring says how. Descriptive, not targets. `python -m tools_py.flow --check` exits 1 when this file is not a render of the commit it names; regenerate at the close.

Rendered at commit `ed519ae5ae05cfa68b24dee1eb05ca5fa9c6de89` (committer time 2026-10-11T23:09:26Z), since 2026-09-01 (UTC days): 3319 commits, 449 merges.

## The numbers

- **merges per day**: 449 merges on 19 days (mean 23.6 a merge day); `git log --merges` counts 449. Per day below -- command: `python -m tools_py.changelog --rev ed519ae5` (its merges, changelog.entries()), by UTC committer day; cross-check: `TZ=UTC git log --merges --since=2026-09-01T00:00:00Z --date=format-local:%F --format=%cd ed519ae5 | sort | uniq -c`
- **fix rounds per merge**: rounds: merges -- 0: 393 (87.5 %); 1: 41 (9.1 %); 2: 9 (2.0 %); 3: 4 (0.9 %); 4: 1 (0.2 %); 7: 1 (0.2 %); none: 87.5 %, at most one: 96.7 %. A round counts once, at the first merge whose range brings it in; merge commits and docs(sprint-N) plan commits are not rounds; rounds committed straight on a sprint branch count at the sprint's merge into main (Sprint 11's seven at PR #49) -- command: `git log --no-merges --format=%s P1..P2 | grep -v -E '^docs\(sprint-[0-9]+\)' | grep -c -E 'review round|fix round'` per merge with parents P1 P2, oldest first, less the commits an earlier merge counted (note 03 section 2.2, deduplicated)
- **fix rounds per merge, broad pattern**: matches: merges -- 0: 220 (49.0 %); 1: 129 (28.7 %); 2: 59 (13.1 %); 3: 14 (3.1 %); 4: 9 (2.0 %); 5: 4 (0.9 %); 6: 1 (0.2 %); 7: 3 (0.7 %); 8: 2 (0.4 %); 9: 1 (0.2 %); 10: 2 (0.4 %); 16: 1 (0.2 %); 19: 1 (0.2 %); 28: 1 (0.2 %); 31: 1 (0.2 %); 32: 1 (0.2 %); none: 49.0 %, at most one: 77.7 % (an upper bound: every fix(scope) commit counts). The same rule -- command: `git log --no-merges --format=%s P1..P2 | grep -v -E '^docs\(sprint-[0-9]+\)' | grep -c -E 'review round|fix round|fix\(|review'` per merge, oldest first, less the commits an earlier merge counted
- **subjects saying " again"**: 59 of 3319 commits -- command: `git log --since=2026-09-01T00:00:00Z --format=%s ed519ae5 | grep -ci -- ' again'`
- **docs share of churn, seven days**: 62.8 %: 9723 of 15488 lines added + deleted, 2026-10-04T23:09:26Z to 2026-10-11T23:09:26Z -- command: `git log --numstat --no-renames --since=2026-10-04T23:09:26Z --until=2026-10-11T23:09:26Z --format= ed519ae5 | awk '$1 ~ /^[0-9]+$/ {t += $1 + $2} $3 ~ /^docs\// {d += $1 + $2} END {print d, t}'`
- **ticket wait**: median 1363 s over 65 tickets -- command: `grep -E 'TICKET [^ ]+ waited [0-9]+' logs/loop_lock.queue.log` (the main tree's), the median of the last field, unstamped lines and lines stamped after 2026-10-11T23:09:26Z or before 2026-09-01 left out
- **open-issue age**: not measured: the open-issues table of docs/BACKLOG.md at `ed519ae5` has no Opened column; the dates live on GitHub (`gh issue list --state open --json number,createdAt`), a network read this page does not make -- command: `git show ed519ae5:docs/BACKLOG.md`, the Opened column of section 1's table, days to 2026-10-11
- **sessions**: 48 distinct Claude-Session trailers -- command: `git log --since=2026-09-01T00:00:00Z --format=%B ed519ae5 | grep '^Claude-Session:' | sort -u | wc -l`
- **commits per session**: largest first: 333, 255, 237, 220, 85, 69, 66, 61, 32, 30, 24, 22, 21, 19, 17, 16, 12, 10, 9, 8, 7, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1; median 5.5 -- command: `git log --since=2026-09-01T00:00:00Z --format=%B ed519ae5 | grep '^Claude-Session:' | sort | uniq -c | sort -rn`
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
| 2026-10-02 | 5 |
| 2026-10-03 | 6 |
| 2026-10-04 | 3 |
| 2026-10-09 | 4 |
| 2026-10-11 | 6 |
