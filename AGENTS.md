# ANTS3 repository guidance for AI coding assistants

When investigating ANTS3 source code, **read [AI_SOURCE_MAP.md](AI_SOURCE_MAP.md) first**. It provides repository-root-relative paths to scripting registration, execution infrastructure and major subsystems.

- Use the source map to **find code**, not as proof of an API contract or runtime behavior.
- Confirm the branch/commit being examined (usually `dev` for ongoing development).
- For a scripting unit, trace its registered name to its C++ interface; inspect the relevant `.h` declarations and matching `.cpp` implementations before making source-dependent claims.
- Check the actual callers, configuration readers/writers, build flags and external dependencies when they matter. Do not invent unsupported scripting methods or configuration fields.
- Report missing evidence explicitly. Distinguish source inspection from successful runtime testing.

For scientific ANTS3 configuration or script generation, use the applicable **ANTS3 AI Knowledge Base** when supplied; the source map does not replace its generation instructions.
