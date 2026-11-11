# Surge patch tag data model

This RFC describes the physical model used to store patch tags and the logical model
used to apply them. It does not specify an implementation or tool.

## Physical model

A patch contains:

- **Filesystem metadata:** its path (currently a `.fxp` file) and its basename without
  the extension. For example, `/foo/bar/This.fxp` has the name `This`.
- **Legacy XML fields:** an author identifier of up to 256 characters and a plain-text
  comment of up to 4096 characters.
- **Categorized tags:** a list of `(category, value)` pairs. Each category can contain
  up to 256 characters; each value, up to 1024. The list has no fixed length, and the
  implementation otherwise places no constraints on UTF-16 categories or values.

## Logical model

To keep flexible key-value tags consistent, define core categories. Each category
specifies whether it may appear once or multiple times and whether its values are
prescribed or free-form.

The following categories are proposed:

### Patch description

Genres are not mutually exclusive or collectively exhaustive (MECE). This proposal is
open to feedback:

- `factory_status`: one value per patch: `factory`, `third_party`, or `user`.
- `category`: one prescribed value per patch. The list is undecided; proposed values
  include `Keys`, `Pads`, and `FX`.
- `sub-category`: zero or more free-form values. For example, a patch in `Keys` might
  have `EP` and `FM` sub-categories.
- `features`: multiple prescribed values. The list is undecided; proposed values include
  `Layered`, `Split`, `Sequenced`, and `MPE`.
- `character`: multiple prescribed values. The list is undecided; proposed values
  include `Cinematic` and `Analog`.

### Extended authorship

- `author_collection`: the name an author gives a patch bank or collection.
- `author_url`: a link to the author's website.
- `author_license`: a license, preferably from <https://spdx.org/licenses/>.

### User tags

`user_tag` may appear any number of times and contain arbitrary values.

## Examples

The factory patch `Keyboards/DX EP.fxp` might have:

- name: DX EP
- author: Claes
- factory_status: factory
- comment: Using Surge's FM capabilities, make an EP like the old DX synths
- category: Keyboards
- sub-category: EP
- sub-category: FM
- character: Digital
- character: Emulation
- author_license: GPL-3.0-or-later
- author_url: <https://surge-synthesizer.github.io/>

A sequenced MPE patch from a project called `ChamberSynth` might have:

- name: That Pad From CS4
- author: baconpaul
- factory_status: user
- comment: In CS4 this is the pad under the piano
- category: Pad
- character: Mysterious
- feature: MPE
- feature: Sequenced
- user_tag: ChamberSynth Patches
- user_tag: CS4
- user_tag: 2020-07 Patches
