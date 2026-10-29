# Tests

Tests protect business rules, external contracts and observable user behavior. They
do not prescribe private class layouts, source spelling, internal object names or
historical implementations.

## Layout

| Directory | Responsibility |
| --- | --- |
| `cases/shared`, `cases/core` | Values, domain invariants and boundaries. |
| `cases/app` | Configuration, startup, commands, paths, staging, enhanced mode and power coordination. |
| `cases/contracts` | WMI transport and UWF provider contracts exercised through controlled transports. |
| `cases/presentation` | ViewModel state, item models, notifications and pagination. |
| `cases/ui` | Real widget input, user flows, readability and optional live captures. |
| `cases/windows` | Native Windows boundaries with read-only queries and owned resources. |
| `cases/composition` | Build, link and use production components independently. |
| `cases/tools` | Positive and negative tests for verification utilities. |
| `tools` | Standalone C++ catalog, image and result utilities; not behavior suites. |
| `support` | Shared input drivers, file-dialog responses, layout inspection and capture support. |

Small fixtures are created inside their tests. Add a `data` directory only when an
independent text fixture is necessary. Do not store screenshots, binaries, logs,
generated manifests or reports in this tree.

Pure string, drive parsing and registry path cases belong to `shared`; snapshot
ordering belongs to `core`. Application path cases exercise containment and
segment boundaries. Native encoding and mounted-volume queries belong to
`windows`, so pure policy suites can link without Windows adapters.

Shared application fixtures are grouped by feature in `support/app` and compiled
in `UwfTestApplicationSupport`. They have no widget dependency, so application
and desktop cases can exercise the same controlled transports and stores.
Shared UI implementations are compiled once in `UwfTestUiSupport` and
`UwfTestVisualSupport`. Suites link only the support they use. Animation checks
advance Qt's animation driver to capture intermediate and final frames while
retaining real widget input and painting. The three command-line tools also have separate implementation
libraries, so their own cases exercise the same code as the executables.

Concrete production classes declaring `Q_OBJECT` include their generated `moc_*.cpp` in
their implementation file. This keeps their meta-object and implementation in
one archive member instead of linking unrelated classes through a combined moc
object. The coordinator and asynchronous-model suites link application or
presentation components without Windows adapters; other suites add adapters
when their tested ports require them. Composition cases also exercise the UWF
provider clients through an injected transport and volume lookup, so their
independent linkage is checked by the linker.

Application UI flows live in `cases/ui/application`. Appearance, shell,
configuration, staging, diagnostics, enhanced mode, and commit/power workflows
have independent QtTest entry points and CTest feature labels. They link
`UwfApplicationUiSupport` for shared fixtures and locale/theme restoration.
Each suite owns its state and retains its relevant transition matrices; individual
methods and data rows remain selectable through QtTest.

Widget behavior is split in `cases/ui/widgets` into appearance, configuration,
controls, pagination, power, and shell suites. `WidgetUiFixture` contains shared
input and measurement helpers only. Configuration lifecycle matrices cover clean
startup, confirmed/canceled discard, unavailable storage, partial writes and
reconciliation. Async matrices cover owner destruction, completion affinity,
nonblocking input, duplicate writes and superseded reads.

Use `qUtf8Printable()` for QString-based QtTest data names and diagnostics.
`qPrintable()` uses the Windows local code page, which can make XML results
invalid when a data name or failure message contains non-ASCII text.

## Build and run

Use the project's configured Windows Clang build with QtTest available:

```text
cmake --build <build-directory>
cmake -E chdir <build-directory> ctest --output-on-failure -j 4
```

`UWF_BUILD_TESTS` controls test registration. The MSVC production configuration
intentionally does not build or run tests because its static Qt has no QtTest;
verify that configuration by building the production application.

CTest labels distinguish the task kind, owning layer, feature and runtime:

```text
ctest -L runtime-appless --output-on-failure -j 4
ctest -L runtime-eventloop --output-on-failure
ctest -L layer-ui --output-on-failure
ctest -L native --output-on-failure
ctest -L kind-check --output-on-failure
```

Run these commands from the build directory. `kind-case` identifies QtTest suites;
`kind-check` identifies artifact validation. Tests of tools remain cases, labeled
`layer-tools`. Native tests use the `native` label as well as their layer label.

Each QtTest CTest entry writes its XML result to
`<build-directory>/test-results/<target>.xml` and keeps readable console output.
Clear or copy that output directory when beginning a separate acceptance run;
old XML files are not evidence that an unexecuted suite passed.

QtTest supports selecting individual methods or data rows directly:

```text
UwfWidgetPowerTests.exe powerConfirmationDefaultsToCancel
UwfStartupConfigurationTests.exe -datatags
```

Desktop tests share the CTest `uwf-desktop` resource lock. This only coordinates
one CTest invocation. Run a single desktop test or capture session at a time,
including when launching executables from an IDE. Pure rules may run in parallel.
The complete application UI suite has a longer timeout because its language,
theme and interaction matrix takes substantially longer than a unit suite.

## Verification tools

`UwfTranslationCatalogCheck` runs Qt `lupdate` and parses TS XML. CTest supplies the
source root, extractor and configured locales. It checks active messages,
translation status, placeholders, rich text, filters and protected technical
text. Application catalogs must contain exactly the extracted source keys;
missing, unused, obsolete and duplicate entries fail validation. Repeated uses
share a key identified by context, source text and disambiguation. Optional
`--source-catalog` accepts an extracted TS, and `--qt-source` checks bundled Qt
message identities against a locally supplied Qt source tree. Bundled Qt catalogs cover
text editing and numeric spin box menus, including bidirectional text controls.
They retain upstream attribution, with locally completed translations validated
by the same completeness and semantic checks as application messages.
No translation service is used.

`UwfImageCompare` compares two fresh capture directories:

```text
UwfImageCompare.exe --reference <before-directory> --actual <after-directory> --report <report.json>
```

An optional `--expected-count` verifies the scene count agreed for that run.
The tool compares complete PNG inventories, decoded RGBA pixels and dimensions
with zero tolerance and no masks. Unreadable images fail even if their file bytes
are identical. Its own tests generate small images in temporary directories.

`UwfTestResultCheck` reads QtTest XML files:

```text
UwfTestResultCheck.exe --report <report.json> <suite-one.xml> <suite-two.xml>
UwfTestResultCheck.exe --expected <migration.json> <suite-one.xml> <suite-two.xml>
```

Without `--expected`, it validates supplied results, not completeness of a run.
An executed function without any recorded outcome is rejected, even when other
functions in that file passed.
For a migration, prepare an independent inventory before changing the cases:

```json
{
  "cases": [
    {"id": "OldTests::requiredBehavior(boundary)", "target": "NewTests::requiredBehavior(boundary)"},
    {"id": "OldTests::implementationDetail()", "target": null, "reason": "No observable contract; superseded by behavior coverage."}
  ]
}
```

Omitting `target` preserves the ID. Renames and deliberate removals are explicit;
unrelated new passes cannot compensate for a missing expected case. The tool
does not infer semantic coverage from source text or prove that an assertion is
well designed. Review the input partitions and oracles during migration. Do not
keep old executable names or row totals as permanent gates.

## Live visual verification

Images are temporary verification output, never application or test resources.
Preserve the reference source/build before a refactoring. Build both versions with
the same version identity and use the same Qt, Windows, fonts, styles, screen/DPR
and fixture paths. Capture both versions when verification is needed; there is no
stored screenshot baseline to download or embed.

`UWF_TEST_SCREENSHOT_DIR` selects an output directory for the existing UI capture
points. `UWF_TEST_VISUAL_FIXTURE_ROOT` enables controlled visual inputs and window
placement. Point it at an owned temporary directory; the file-dialog case creates
its empty `file-dialog` subdirectory. Keep visible paths the same in both runs,
and run the same methods and data rows in the same conditions.

Include before/after actions, focus, hover, pressed, checked/unchecked,
selected/unselected, disabled, menus, pagination and errors. A screenshot is
rendering evidence; it does not replace the corresponding behavior assertion.
Compare the full agreed scene set and keep the report. Delete images after
review when they are no longer needed. A future intentional UI improvement uses
the new intended appearance instead of inheriting a permanent old-image gate.

Capture support requires stable widget and native compositor frames. Use the
same static desktop background behind both versions: native frame captures
include the desktop beneath rounded window corners. The native progress-style
clock is specific to the supported Qt Windows plugin; unsupported capture
conditions must be reported rather than treated as successful verification.

## Adding or changing coverage

1. Identify the rule or user behavior and its owning layer.
2. List normal, boundary, unavailable and failure outcomes, including relevant
   cancellation, repeat, interleaving and destruction sequences.
3. Use independent expected results. Preserve complete finite-state transition
   tables where they already exist; use deterministic properties for numeric and
   path invariants.
4. Test the rule in its owning layer, notifications/mapping in presentation,
   wiring and affordances in UI, and actual system facts in native tests.
5. Restore modified global state with local RAII, even after an assertion fails.
   Wait for signals or conditions with bounded waits rather than arbitrary sleeps.
6. Register the suite through `uwf_add_test` with its layer, feature and runtime.

To add an independent suite, add its sources and registration in
`cases/CMakeLists.txt`; select it with CTest labels or build its target directly.
For an existing application flow, add its slot and implementation to the owning
feature unit. Additional units are linked explicitly. Helpers and tools have no
central class-name registry or runtime plugin loader. Removing a suite or support
dependency is a normal CMake target change.

A defect should extend the relevant input partition or state transition, not
create an unrelated issue-number suite. Extract shared fakes only when callers
need the same behavior. Keep specialized failure responses local.

Coordination coverage includes synchronous completion, coalescing during start,
startup failure, external reservations, reentrant observers and destruction.
Enhanced-mode application cases distinguish the service contract from agent
authentication and cover status-read failure, power acknowledgements and owned
or borrowed connection lifetimes. Desktop cases verify their presentation and
action routing. Text rendering cases preserve literal user data and require
callers to opt into rich text explicitly.
Enhanced-mode sessions cover service-contract gating, connection ownership,
authentication notifications, query failures and preshutdown acknowledgements
without widgets or Windows adapters. Configuration-import cases also exercise
targets removed during synchronous notifications and between commands.
Background-model coverage includes superseded success/failure, notification
reentrancy, recovery, pending delivery at destruction and model thread affinity.
These cases run with `QCoreApplication` and injected sources. Registry metadata
has its own suite, separate from command framing and acknowledgement protocols.

Build dependencies and independent component linking protect composition. There
are no source-scanning architecture tests or class/directory-name allowlists.
Routine refactoring may change private names, control nesting, legal internal
call sequences and font implementation flags without breaking behavior tests.

Automatic system tests do not change live UWF settings, install the production
service or shut down Windows. Controlled transports prove requested operations
and failure handling; actual privileged system changes need a separate dedicated
environment. Missing capabilities or incomplete runs must remain visible in the
acceptance report.
