#!/usr/bin/env python3
"""Check the approved INI catalog and its MkDocs reference; no Unreal required.

INI rows are the registration source of truth. APPROVED stores name-set contract
fingerprints, not a second inventory. Hash sorted UTF-8 names joined by LF with a
trailing LF. Change these only for an explicitly reviewed catalog revision; see
docs/bertagascompanionext/gameplay-tags/extension-guide.md.
"""

from collections import Counter, defaultdict
import hashlib
from pathlib import Path
import re
import sys


EXPECTED_TOTAL = 474
# Family: (explicit count, SHA-256 of the approved canonical name set).
APPROVED = {
    "Ability": (52, "8d9340701954166f80c760efa591957d5daa2ec234050bb0afc942d522481d88"),
    "State": (51, "0676c1f1b535f37fb80596761213c106a63f853b320373dde831057cad746e66"),
    "Status": (23, "d4f92b0f1f04e469fbbd5bfe3d7d25984774b266124dc9b681da58557931ba1a"),
    "Event": (50, "e11c69fcc62f6ef2580f6c20e931de9bfc9ad688253c2a62e8f74b7a5d46aed8"),
    "Cooldown": (48, "86094fad4e1063f5206893c43b5464a7117935ab59dd80dc81114c06ea8a1979"),
    "Damage": (23, "312df77a13ce4177aa558151fe8e4dc32324f5dff7e9351d5803b9fdba88ced2"),
    "Effect": (8, "a0cc04053ca117669cefcbef39eb784ecd3417d16637a052eaa2a938cfe75466"),
    "Resource": (10, "e28c39d033942607b55dcb51c51efe11e391e48e74ecc823b035dbc1de7ea65f"),
    "Input": (17, "b0b1d0ef37a4db0cc5302809dc3ac8b834ef64c1f0340d5d657a969e565963ad"),
    "Interaction": (16, "ee76ee6a09bdf5b4b44ea11f508e60bd475e4e40447586be25392a71967f1885"),
    "Equipment": (12, "f0853d7d5a0f799118dd7dd455d2741bd4a5a499c344287fc2c1204b2b5c959a"),
    "Weapon": (18, "adf94ea05e665c20c9886ffe8b459cee345c4d9a36d85826862989cb70d70431"),
    "Item": (16, "1832b7a71aa0d121fedf144a26408a8f9d0fb9dc0df2faeb1df4eebec99cc631"),
    "Inventory": (8, "57e6e09bf6a9f4563988007223c85fbd321000d6d036ac28c6c4a8b0f4c3b72f"),
    "Targeting": (18, "65dbb8784e3ec4bd067d8f099004d3f7a798dc29ad492af5e2e145b4c57b19f8"),
    "AI": (17, "a100a7f28443490c9731f4217a4aa81fa7e10a5c66e94647cbbd4af6b585c60f"),
    "Quest": (11, "5690045ba6c1f594de21bb925a8a7d8711693a2324209270be9914b5f30ffcf4"),
    "Progression": (6, "315e0d9b9bb880d24644d586213eae2dea7e327e822ddd2c3b4ed68e931f5f63"),
    "World": (19, "b69cea10e88d069118f7dcb49b5bdfb50e90b14ff818b9fd97f484b6f7482bfb"),
    "Vehicle": (10, "650072bf7311a41e227ccdcdbe301170ff23541ac49cd6cf36e2467eec906e69"),
    "Camera": (8, "098a7ac69ee5658e8ccce970ce8aceb0862e1cbe8b2b4374860ac83d1b5d6fe9"),
    "UI": (13, "cc1dff20ae1ef976f0eb81a2a3bd2b7c0ea1790807ca2c13e68fffbcdf4893ce"),
    "GameplayCue": (20, "35f9550dc3512835aa142e17dfeeba1a0af51f94c184c3744519cfd36c7e73fc"),
}
SECTION = "[/Script/GameplayTags.GameplayTagsList]"
ROW = re.compile(r'\+GameplayTagList=\(Tag="([^"\\]+)",DevComment="([^"\\]*)"\)')
NAME = re.compile(r"(?:Berta\.[A-Z][A-Za-z0-9]*|GameplayCue\.Berta)(?:\.[A-Z][A-Za-z0-9]*)+")
HEADING = re.compile(r"^### (\S+)\s*$", re.MULTILINE)
# Include unknown families/case to catch misspelled references, not just known names.
REFERENCE = re.compile(r"\b(?:Berta|GameplayCue\.Berta)(?:\.[A-Za-z][A-Za-z0-9]*)+")
FIELDS = ("Meaning", "Scope and suggested owner", "Lifecycle", "Example", "Implementation responsibility")


def family_of(name):
    return "GameplayCue" if name.startswith("GameplayCue.Berta.") else name.split(".")[1]


def page_for(family):
    return ("gameplay-cue" if family == "GameplayCue" else family.lower()) + ".md"


def validate(root):
    """Return deterministic findings, counts, and dedicated-entry count."""
    errors = []
    source_dir = root / "BertaDevKitHost/Plugins/BertaGASCompanionExt/Config/Tags"
    docs_dir = root / "docs/bertagascompanionext/gameplay-tags"
    sources = sorted(source_dir.rglob("*.ini"))
    if not sources:
        errors.append("No catalog INI sources found")
    names = []
    comments = []
    families = defaultdict(list)
    source_basenames = Counter(p.name.casefold() for p in sources)
    for basename, count in sorted(source_basenames.items()):
        if count != 1:
            errors.append(f"Duplicate source basename: {basename}")
    for path in sources:
        section_count = 0
        source_names = []
        for number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            line = raw.strip()
            context = f"{path.relative_to(root).as_posix()}:{number}"
            if not line or line.startswith(";"):
                continue
            if line == SECTION:
                section_count += 1
                continue
            match = ROW.fullmatch(line)
            if match is None or section_count != 1:
                errors.append(f"{context}: invalid catalog syntax/section")
                continue
            name, comment = match.groups()
            names.append(name)
            source_names.append(name)
            comments.append(comment.strip())
            if not NAME.fullmatch(name) or name.startswith("Berta.GameplayCue."):
                errors.append(f"{context}: invalid canonical namespace/name: {name}")
            else:
                families[family_of(name)].append(name)
            # Structural quality checks; semantic usefulness still requires review.
            if len(comment.strip()) < 35 or len(comment.split()) < 5:
                errors.append(f"{context}: DevComment needs a substantive description: {name}")
            if comment.strip().casefold() in {"todo", "placeholder", name.casefold(), name.split(".")[-1].casefold()}:
                errors.append(f"{context}: placeholder DevComment: {name}")
        if section_count != 1:
            errors.append(f"{path.name}: expected exactly one GameplayTagsList section")
        if source_names != sorted(source_names):
            errors.append(f"{path.name}: entries must be sorted by canonical name")
    for name, count in sorted(Counter(n.casefold() for n in names).items()):
        if count != 1:
            errors.append(f"Duplicate explicit declaration (case insensitive): {name}")
    for comment, count in sorted(Counter(comments).items()):
        if count != 1:
            errors.append(f"Repeated DevComment ({count} entries): {comment}")
    if len(names) != EXPECTED_TOTAL:
        errors.append(f"Expected {EXPECTED_TOTAL} explicit entries, found {len(names)}")
    if set(families) != set(APPROVED):
        errors.append(f"Family mismatch: missing={sorted(set(APPROVED) - set(families))}, unexpected={sorted(set(families) - set(APPROVED))}")
    for family, (expected_count, expected_hash) in APPROVED.items():
        actual = sorted(families[family])
        if len(actual) != expected_count:
            errors.append(f"{family}: expected {expected_count}, found {len(actual)}")
        digest = hashlib.sha256(("\n".join(actual) + "\n").encode("utf-8")).hexdigest()
        if digest != expected_hash:
            errors.append(f"{family}: approved name fingerprint mismatch; missing, unapproved, or renamed entries")

    registered = set(names)
    documented = []
    catalog_dir = docs_dir / "catalog"
    reference_pages = sorted(catalog_dir.glob("*.md"))
    expected_pages = {page_for(f) for f in APPROVED}
    if {p.name for p in reference_pages} != expected_pages:
        errors.append("Reference page set does not match approved families")
    for path in reference_pages:
        text = path.read_text(encoding="utf-8")
        headings = list(HEADING.finditer(text))
        for i, heading in enumerate(headings):
            name = heading.group(1)
            documented.append(name)
            if name not in registered or NAME.fullmatch(name) is None:
                errors.append(f"{path.name}: orphaned/invalid dedicated entry: {name}")
                continue
            if path.name != page_for(family_of(name)):
                errors.append(f"{path.name}: entry in wrong family: {name}")
            end = headings[i + 1].start() if i + 1 < len(headings) else len(text)
            body = text[heading.end():end]
            for field in FIELDS:
                field_text = re.search(r"\*\*" + re.escape(field) + r":\*\* ([^\n]+)", body)
                minimum_words = 3 if field == "Scope and suggested owner" else 5
                if field_text is None or len(field_text.group(1).split()) < minimum_words:
                    errors.append(f"{path.name}: {name}: missing/substantive field required: {field}")
        family = next((f for f in APPROVED if page_for(f) == path.name), None)
        if family and not text.startswith(f"# {family} — {APPROVED[family][0]} tags\n"):
            errors.append(f"{path.name}: family heading count is out of date")
    for name, count in sorted(Counter(documented).items()):
        if count != 1:
            errors.append(f"Duplicate dedicated documentation entry: {name}")
    for name in sorted(registered - set(documented)):
        errors.append(f"Missing dedicated documentation entry: {name}")

    # Ancestors are valid dictionary references, but never dedicated catalog entries.
    dictionary = set(registered)
    for name in registered:
        parts = name.split(".")
        dictionary.update(".".join(parts[:i]) for i in range(1, len(parts)))
    for path in sorted(docs_dir.rglob("*.md")):
        for name in sorted(set(REFERENCE.findall(path.read_text(encoding="utf-8")))):
            # Explicitly discussed prohibited prefix, not a canonical entry/reference.
            if name == "Berta.GameplayCue":
                continue
            if name not in dictionary:
                errors.append(f"{path.relative_to(root).as_posix()}: unregistered canonical reference: {name}")
    return errors, {f: len(families[f]) for f in APPROVED}, len(documented)


def main():
    root = Path(__file__).resolve().parents[1]
    try:
        errors, counts, documented = validate(root)
    except (OSError, UnicodeError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(f"PASS: {sum(counts.values())} approved explicit tags, {len(counts)} families, {documented} dedicated documentation entries")
    print("PASS: INI structure, namespaces, unique names/comments, ordering, fingerprints, and documentation references")
    print("Counts: " + ", ".join(f"{family}={count}" for family, count in counts.items()))
    print("Static evidence only; Unreal registration and runtime behavior require separate verification.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
