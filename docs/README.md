# docs

The demo shown in the main [README](../README.md) is an **MP4 uploaded directly
through GitHub's web interface**, not a file stored in this repository.

To add or replace it: edit `README.md` on github.com (or in a pull request) and
drag the `.mp4` onto the `<!-- VIDEO -->` marker near the top. GitHub hosts the
file on its own CDN and inserts a player link automatically — nothing is
committed to the repo.

This keeps large media out of git history. Local `*.mp4` / `*.gif` files are
gitignored so they cannot be committed by accident.
