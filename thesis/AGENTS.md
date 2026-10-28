# Notes for agents

- The thesis is English (Olav Schettler, 1995), the Stellingen are Dutch and English. Never "improve" the author's wording, spelling or punctuation; only fix conversion errors.
- `src/chapters/*.adoc` are the source of truth. `make convert` regenerates them from the PDF and overwrites manual fixes.
- Use the Docker images for tools (`eda-tools`, `asciidoctor/docker-asciidoctor`); run Python only via `uv run`.
- Do not use emoji or UTF-8 icons.
