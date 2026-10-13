# Storyline groups

A **storyline group** collects entries that continue, expand, or retell one central story. It sits between an individual season/movie and the broader franchise or shared universe. Keep the existing `ip_group` field as its database link; no new field is needed.

## Membership rules

1. Direct sequel seasons, split cours, and concluding movies stay together.
2. Specials, OVAs, compilation movies, and retellings stay with the story they supplement. A change of format, viewpoint, streaming listing, or title does not create a new group.
3. A prequel stays when it develops that central cast's journey. A self-contained predecessor with a new central cast and its own complete dramatic arc can have a separate group.
4. A spin-off gets its own group when it has a distinct protagonist/central ensemble and an independently sustained story. Shared setting, lore, cameos, branding, or chronological proximity alone do not justify a merge.
5. A temporary protagonist change within a continuing plot does not split a group.
6. For substantially different alternate routes or reboots, keep a separate storyline until reviewed. Minor adaptation differences and a retelling of the same core adventure do not require a split. This is a collection policy, not a claim that every entry shares exact canon continuity.

Practical question: **Is this the next chapter, another view, or a supplement to this central story—or a different story that happens to share its world?**

## Names and examples

Use the shortest recognizable title of the central story. Add a subtitle only to distinguish an independent branch. Do not put `Group`, `Franchise`, season numbers, or individual movie titles into the group name unless needed to distinguish the story.

| Group | Included | Kept separate |
|---|---|---|
| Sword Art Online | Kirito/Asuna's seasons, Alicization, Extra Edition, Progressive films; other qualifying main-story films | Sword Art Online Alternative: Gun Gale Online |
| Sword Art Online Alternative: Gun Gale Online | LLENN's story and its sequel seasons | Kirito's Gun Gale arc in SAO II stays in Sword Art Online |
| SPY x FAMILY | The TV seasons and Code: White | Any future independent spin-off would be reviewed separately |
| Fate/stay night: Heaven's Feel | All three Heaven's Feel films | Unlimited Blade Works, Fate/Zero, Fate/Grand Order, strange Fake remain separate in this pass |
| Fate/Grand Order | The qualifying Babylonia and Solomon entries | Other Fate storylines |

The SAO distinction follows the user's requested policy. Official character pages establish the separate central casts: [Progressive](https://saop-anime.com/aria/story-character/character/) and [Alternative GGO](https://gungale-onlineusa.com/1st/character/). The other applied merges are supported by the downloaded catalog's episode descriptions. These are curated membership decisions, not automatic deductions from title prefixes.

## Implementation and review

`scripts/crunchyroll-storylines.json` records explicit merges and reasons. Unreviewed imports keep their source-series grouping as a provisional starting point. The current pass does not claim to have resolved every adaptation, route, paired film, or spin-off. Never merge on a shared first word such as `Fate`.

The membership rule and the 80% viewing rule are separate: membership says where an entry belongs; viewing coverage says whether it belongs in this import. Do not invent missing seasons or movies to fill a group.

Ranks belong to the entity originally ranked. Copy an entry's explicit rank only to its matching entry; do not spread it across sequels or infer a group rank. The user confirmed that zero means unranked, not a rank derived from row order. In the source files, only `Eighty Six 86` has nonzero rank (1), mapped explicitly to `86 EIGHTY-SIX`. The original group file has no ranked groups.

## Schema compatibility

The imports preserve all 15 original entry columns and all 15 original group columns, in their original order. The original entry header's legacy numeric type tags are incompatible with the current enum; the generated entry header uses current type tags while retaining the field layout. The group header retains the original tags.

Unknown optional scalar fields are blank, optional text is `""`, and empty vectors are `{}`. Required group format-dependent counts are zero placeholders until entry formats are classified. Known runtime totals and membership remain populated. Missing values load into the current structs' defaults; the structs do not preserve an unknown-value flag, so a load/save cycle may materialize those defaults. Blank fields must not be interpreted as researched release status, format, genre, or fan-service ratings.

The parser treats empty non-text fields as missing values, allowing database handlers to reject missing required fields and skip unknown optional fields. `runtimeComplete` is left blank because its existing schema tag is U64 while its C++ member is bool. Group `genres` is also left blank: its existing schema tag is VECU8 but its member is VECU32. Leaving these optional fields missing preserves the original group schema without inventing values or changing those unrelated type mismatches.
