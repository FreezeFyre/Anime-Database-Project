# Crunchyroll database pair

271 qualifying entries and 180 storyline groups in ADP0 version 1002 format. Load these files together as a separate dataset. IDs start at zero and are local to this pair; do not append to the originals without remapping IDs. Original databases are unchanged.

Both files contain all 15 original fields, in the original order. Entry type tags use the current enum because the original entry header has obsolete tags. The group schema is identical to the original. Unknown optional scalars are blank, empty text is quoted, and empty vectors are {}. Requires the accompanying parser fix for blank non-text fields. Loaded unknown values still become C++ defaults; the structs do not retain missing-value flags. Required episode/movie/series group counts are zero placeholders until formats are classified; runtimes and membership are populated.

Grouping policy: see ../../docs/storyline-groups.md. Explicit merges are reproducible in scripts/crunchyroll-storylines.json; default source-series groupings are provisional where not reviewed. SAO and Progressive form one group; Alternative GGO remains separate.

Rank transfers: Eighty Six 86 → 86 EIGHTY-SIX (rank 1). Other entries and all groups are unranked. No row-order rankings were inferred.

Locales without existing lookup values: ar-SA, ca-ES, de-DE, hi-IN, id-ID, it-IT, ms-MY, pl-PL, pt-BR, pt-PT, ru-RU, th-TH, tl-PH, tr-TR, vi-VN. Source JSON retains them.

The parser has no escaped-quote support. Display quote substitutions:
- The Most Notorious "Talker" Runs the World's Greatest Clan — Season 01 → The Most Notorious “Talker” Runs the World's Greatest Clan — Season 01
- The Most Notorious "Talker" Runs the World's Greatest Clan → The Most Notorious “Talker” Runs the World's Greatest Clan
