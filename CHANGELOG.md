# Changelog

## 2026-10-28

### EPUB edition of the thesis

The FrameMaker sources are lost, so the thesis is converted from `thesis/thesis.pdf` via AsciiDoc to EPUB 3 (build setup as in *Systemic Relativism*).

- `thesis/scripts/normalize.py` splits the 4-up pages (chapters 2-4, references, acronyms, appendix) into single book pages.
- `thesis/scripts/pdf2adoc.py` converts headings, emphasis, code listings, footnotes, citations, figures and tables to AsciiDoc; `thesis/scripts/qa_words.py` checks the vocabulary against the PDF.
- `thesis/src/` holds the proofread AsciiDoc sources, figures and cover; manual fixes include Table 4, the summary table in Section 7.5 and listings split at page breaks.
- `thesis/build/` builds EPUB and HTML with Docker (`make epub`, `make validate`, `make html`); results are in `thesis/output/`.

## 2015-08-21

### Added thesis in PS and PDF

PostScript files per chapter and the combined PDF in `thesis/`.

## 2014-11-30

### Corrected references

### The software

The Cocos libraries and applications: Tcl binding, the copag parser generator, the VHDL parser and examples.

### Reference to http://encapsulate.tripod.com/

### Initial commit
