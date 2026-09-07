# Development

## Repository architecture

`BertaDevKitHost` is the development and verification harness. It contains two independent plugin roots:

```text
BertaDevKitHost/
├── BertaDevKitHost.uproject
└── Plugins/
    ├── BertaDevKit/
    └── BertaDualSense/
```

Both plugins target Unreal Engine 5.8. BertaDualSense is Win64-only.

## Build the host

The primary Editor development target is:

```text
BertaDevKitHostEditor Win64 Development
```

For example:

```text
<UE_5.8>/Engine/Build/BatchFiles/Build.bat BertaDevKitHostEditor Win64 Development -Project="<repo>/BertaDevKitHost/BertaDevKitHost.uproject" -WaitMutex
```

Run verification appropriate to the change. A successful C++ build alone does not prove Editor, Blueprint, visual, runtime-device, or packaged behavior.

## Documentation site

Install the pinned documentation dependency and build the site from the repository root:

```text
python -m pip install -r requirements-docs.txt
mkdocs build --strict
```

For local authoring:

```text
mkdocs serve
```

The generated `site/` directory is ignored. GitHub Actions builds `site/` with the same strict command and deploys its artifact to GitHub Pages; it does not use a `gh-pages` branch.
