# AI Usage Documentation – Phase 1 Filter Command

## Tool Used
**Claude (Anthropic)** – claude.ai (Claude Sonnet 4.6)

---

## What Was Generated

### parse_condition
A function using `strchr()` to find the first and second colon separators, then `strncpy()` to extract each part into the output buffers.

### match_condition
A function that checks the field name with `strcmp()`, then converts the value to the appropriate C type before comparing:
- `severity` → converted with `atoi()`
- `timestamp` → converted with `atol()` cast to `time_t`
- `category` and `inspector` → compared as strings with `strcmp()`

---

## What I Changed and Why
- The AI used `Report` as the struct type name, but my struct is `Report_t` — updated all references.
- The AI left out the `!=` operator for numeric fields — added those cases manually.

---

## What I Learned
- `strchr()` is useful for splitting strings without modifying the original.
- `strncpy()` requires careful null terminator handling.
- AI-generated code needs careful review — the struct name mismatch and missing operators would have caused bugs.

---

## Critical Evaluation
The logic was sound but the AI assumed a struct name that didn't match my code and omitted the `!=` operator for numeric comparisons. It also doesn't handle unsupported operators explicitly — it just returns 0 silently. Minor issues but confirms AI output always needs human review.