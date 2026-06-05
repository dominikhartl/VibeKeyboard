# `case/` — local design assets (not published)

This folder is your **local drop-spot** for personal design files:

- a **logo** image (e.g. `logo.png`, `logo.svg`)
- a **case / enclosure STL** (e.g. `vibekeyboard-case.stl`)
- any related CAD source (`.step`, `.3mf`, etc.)

## These files are git-ignored on purpose

Everything in this folder **except this README** is excluded from git (see the repo
[`.gitignore`](../.gitignore)). So you can keep your logo and printable model here without
pushing them to GitHub. They stay on your machine.

```gitignore
case/*
!case/README.md
```

## Want to publish your case instead?

If you'd like to share your enclosure with others, either:

- remove the `case/*` lines from `.gitignore` and commit your files here, or
- attach the STL to a GitHub Release.

## Printing notes (fill in for your own design)

- Material: _e.g. PLA / PETG_
- Layer height: _e.g. 0.2 mm_
- Switch cutouts: 14 × 14 mm for standard Cherry MX
- Remember clearance for the Pro Micro's USB connector
