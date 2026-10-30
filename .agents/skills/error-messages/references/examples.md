# Error Message Examples

These examples combine guidance from Wix UX, Google Technical Writing, Google AIP-193, and Material Design.

## UI Modal

Weak:

```text
Whoops! Something went wrong.
The third-party you are trying to connect to is not responding, so we can't fetch your data. Try again later.
```

Problems:

- cute tone
- passes blame to another provider
- uses implementation wording like "fetch"
- gives no confidence about saved state
- offers no fallback if retry fails

Better:

```text
Unable to connect your account
Your changes were saved, but we could not connect the account because the provider is unavailable. Try again later, or contact support if this keeps happening.
```

Why it works:

- says what happened
- explains why at the right level
- reassures the user
- offers a fix and fallback

## Inline Form Validation

Weak:

```text
Invalid postal code.
```

Better:

```text
Enter a 5-digit ZIP code
```

Better when the surface can show more detail:

```text
The ZIP code must be 5 digits. You entered 4872953.
```

Do not show the error before the user interacts with the field or submits the form. Preserve the user's input so they can fix it.

## Permissions

Weak:

```text
Permission denied.
```

Better for an end-user UI:

```text
You need billing access to download invoices. Ask an account owner to update your role.
```

Better for an API:

```json
{
  "error": {
    "code": 403,
    "status": "PERMISSION_DENIED",
    "message": "You need billing.viewer access to download invoice inv_123.",
    "details": [
      {
        "@type": "type.googleapis.com/google.rpc.ErrorInfo",
        "reason": "MISSING_REQUIRED_ROLE",
        "domain": "billing.example.com",
        "metadata": {
          "resource": "invoices/inv_123",
          "requiredRole": "billing.viewer"
        }
      }
    ]
  }
}
```

## Quota Or Limit

Weak:

```text
Quota exceeded.
```

Better:

```text
You requested 2 CPUs, which exceeds your quota of 1 CPU in us-central-1. Request a quota increase or choose a region with available quota.
```

## CLI Error

Weak:

```text
Could not read config.
```

Better:

```text
Cannot read config file at /app/config.yml: permission denied. Give the current user read access or pass --config with a readable file.
```

Code shape: the `⇥` payload carries the facts, and the CLI boundary writes the sentence once.

```faber
union ConfigError {
    NotReadable {
        string path
        string reason
    },
    Missing {
        string path
    }
}

fn config_message(ConfigError e) → string {
    match e {
        case NotReadable let path, reason {
            return "Cannot read config file at §: §. Give the current user read access or pass --config with a readable file."(path, reason)
        }
        case Missing let path {
            return "No config file at §. Create it, or pass --config with an existing file."(path)
        }
    }
}
```

The `faber` CLI is a good model for developer-facing errors. A failing `faber check` writes each diagnostic to stderr with a stable `CODE:slug`, the phase, file, span, source line, message, and a `help` sentence, ends with a `faber explain` pointer, and exits 65. First block and footer, with the absolute path abbreviated:

```text
error[SEM010:expression_type_mismatch] analysis <pkg>/src/main.fab
phase: analysis
file: <pkg>/src/main.fab
span: 25..29
source:     let int x ← "no"
message: expression type mismatch
help: provide a value of the required type, or convert it with `↦` when supported
...
explain: faber explain SEM010.expression_type_mismatch
```

## API Validation Payload

Prefer distinct, structured violations over one generic validation message.

Weak `400` response body:

```json
{
  "error": "Invalid request"
}
```

Better `400` response body:

```json
{
  "error": {
    "code": "INVALID_ARGUMENT",
    "message": "The request has 2 invalid fields.",
    "details": [
      {
        "reason": "FIELD_VIOLATION",
        "fieldViolations": [
          {
            "field": "email",
            "description": "Enter an email address that includes @ and a domain."
          },
          {
            "field": "seatCount",
            "description": "Seat count must be between 1 and 50. You entered 72.",
            "context": {
              "min": 1,
              "max": 50,
              "actual": 72
            }
          }
        ]
      }
    ]
  }
}
```

## Separate Project Example: Billing CSV Import

Scenario: a SaaS billing app lets account admins upload a CSV of customer invoices.

Before:

```text
Upload failed. Invalid file.
```

```text
Bad date.
```

```json
{
  "error": "CSV validation failed"
}
```

Problems:

- generic
- no row or field location
- no preserved-state reassurance
- no constraint
- no stable machine-readable detail for branching or automation

Better UI summary:

```text
We couldn't import 12 invoices because some rows need fixes. Your file is saved as a draft.
```

Better inline row error near `due_date`:

```text
Due date must be after the invoice date. Change row 8 to a date after May 14, 2026.
```

Recovery actions:

```text
Fix rows in draft
Download error report
Cancel import
```

Better API response:

```json
{
  "code": "invoice_import_validation_failed",
  "message": "Some invoice rows need fixes before import.",
  "details": [
    {
      "code": "due_date_before_invoice_date",
      "row": 8,
      "field": "due_date",
      "message": "Due date must be after the invoice date.",
      "context": {
        "minExclusive": "2026-05-14"
      }
    }
  ],
  "context": {
    "draftImportId": "imp_7x92",
    "uploadedFileName": "may-invoices.csv",
    "rejectedRows": 12,
    "availableActions": ["edit_draft", "download_report"]
  }
}
```

Code representation of one detail. Each kind of detail gets its own typed context class, not an open map:

```faber
@ json { }
class DueDateContext {
    @ json { name = "minExclusive" }
    let string min_exclusive
}

@ json { }
class ImportErrorDetail {
    let string code
    let int row
    let string field
    let string message
    let DueDateContext context
}

fn due_date_before_invoice_date_error(int row, string invoice_date) → ImportErrorDetail {
    let DueDateContext limit ← DueDateContext { min_exclusive = invoice_date }
    let string fix ← "Due date must be after the invoice date."
    return ImportErrorDetail { code = "due_date_before_invoice_date", row = row, field = "due_date", message = fix, context = limit }
}
```

A UI renders `message` in an alert region and each detail next to its row and field.

Why it works:

- says what happened: some invoice rows could not be imported
- says why: the due date violates a specific constraint
- says what was preserved: the upload is saved as a draft
- says how to fix it: edit row 8 and choose a due date after the invoice date
- gives a way out: download the report or cancel the import
- keeps prose human-facing while giving machines stable `code`, `field`, `row`, and contextual constraint values

## Boundary Payload Pattern

Use a small payload at presentation boundaries rather than making every internal error type carry every possible field. In Faber, the payload is a set of concrete `@ json` classes, and `↦ json` serializes it:

```faber
import from "norma:json" json

@ json { }
class ViolationContext {
    let int row
    @ json { name = "minExclusive" }
    let string min_exclusive
}

@ json { }
class FieldViolation {
    let string field
    let string reason
    let string message
    let ViolationContext context
}

@ json { }
class ImportContext {
    @ json { name = "draftImportId" }
    let string draft_import_id
    @ json { name = "uploadedFileName" }
    let string uploaded_file_name
    @ json { name = "rejectedRows" }
    let int rejected_rows
    @ json { name = "availableActions" }
    let list<string> available_actions
}

@ json { }
class ImportErrorPayload {
    let string code
    let string message
    let list<FieldViolation> details
    let ImportContext context
}

main {
    let ViolationContext at_row ← ViolationContext { row = 8, min_exclusive = "2026-05-14" }
    let string fix ← "Due date must be after the invoice date."
    let FieldViolation due_date ← FieldViolation { field = "due_date", reason = "before_invoice_date", message = fix, context = at_row }
    let list<string> actions ← ["edit_draft", "download_report"]
    let ImportContext draft ← ImportContext { draft_import_id = "imp_7x92", uploaded_file_name = "may-invoices.csv", rejected_rows = 12, available_actions = actions }
    let string summary ← "We couldn't import 12 invoices because some rows need fixes. Your file is saved as a draft."
    let ImportErrorPayload error ← ImportErrorPayload { code = "invoice_import_validation_failed", message = summary, details = [due_date], context = draft }
    let json encoded ← error ↦ json
    print json.stringify(encoded)
}
```

`faber run` prints one line with keys in sorted order (wrapped here):

```json
{"code":"invoice_import_validation_failed","context":{"availableActions":["edit_draft","download_report"],
"draftImportId":"imp_7x92","rejectedRows":12,"uploadedFileName":"may-invoices.csv"},"details":[{"context":
{"minExclusive":"2026-05-14","row":8},"field":"due_date","message":"Due date must be after the invoice date.",
"reason":"before_invoice_date"}],"message":"We couldn't import 12 invoices because some rows need fixes. Your file is saved as a draft."}
```

Use `details` for repeated or item-specific repair facts. Use `context` for facts that apply to the whole error. Do not add optional top-level fields unless the API already standardizes them.

Faber 1.17 does not allow a generic `ErrorPayload<D>` with a `list<D>` field or an open `json` context field in an `@ json` class (`SEM018:json_genus_field_type_unsupported`). Declare one payload class per boundary instead. Outside `main`, `↦ json` is failable and needs a handler or a `⊥` value (`SEM011:failable_conversio_requires_handler`).

## Faber Boundary Example

Keep domain errors typed and render them at the edge.

Weak: a string payload makes the message the contract.

```faber
fn create_workspace(string name) → string ⇥ string {
    require name.trim() ≢ "" throw "validation failed"
    reject name_taken(name) throw "name taken"
    return name
}
```

Every caller that wants to tell these failures apart has to compare `err ≡ "name taken"`. Rewording the message breaks those callers silently, and the field-level repair data is gone.

Better: a `union` carries the facts, an `@ conversion` maps it to the CLI boundary payload, and the entry point writes to stderr and sets the exit code.

```faber
class FieldViolation {
    let string field
    let string reason
    let string message
}

union WorkspaceError {
    Invalid {
        list<FieldViolation> violations
    },
    NameTaken {
        string name
    }
}

fn name_taken(string name) → bool {
    return name ≡ "engine"
}

fn create_workspace(string name) → string ⇥ WorkspaceError {
    if name.trim() ≡ "" {
        let FieldViolation required ← FieldViolation { field = "name", reason = "required", message = "Enter a workspace name." }
        throw variant Invalid { violations = [required] }
    }
    reject name_taken(name) throw variant NameTaken { name = name }
    return name
}

class CliError {
    let string code
    let string message
    let list<FieldViolation> details
    let int exit_code
}

@ conversion
fn cli_error_from_workspace(WorkspaceError e) → CliError {
    match e {
        case Invalid let violations {
            let string summary ← "Workspace details need fixes."
            return CliError { code = "workspace_validation_failed", message = summary, details = violations, exit_code = 64 }
        }
        case NameTaken let name {
            let string taken ← "Workspace name \"§\" is already in use. Choose a different name."(name)
            return CliError { code = "workspace_name_taken", message = taken, details = empty, exit_code = 65 }
        }
    }
}

fn create_command(string name) → string ⇥ CliError {
    return create_workspace(name)
}

fn run(string name) → int {
    do {
        print "Created workspace §."(create_command(name))
        return 0
    }
    catch err {
        warn "error[§]: §"(err.code, err.message)
        for from err.details let d {
            warn "  §: §"(d.field, d.message)
        }
        return err.exit_code
    }
}

@ cli { name = "workspace" }
@ operand { type = string, binding = name }
main args argv exit run(argv.name) {
}
```

Real behavior under `faber run`:

```text
$ faber run . -- docs        # stdout, exit 0
Created workspace docs.

$ faber run . -- ""          # stderr, exit 64
error[workspace_validation_failed]: Workspace details need fixes.
  name: Enter a workspace name.

$ faber run . -- engine      # stderr, exit 65
error[workspace_name_taken]: Workspace name "engine" is already in use. Choose a different name.
```

Why it works:

- `create_command` propagates with a bare call. The `@ conversion` translates the domain payload, so the domain layer never formats CLI text.
- The renderer has no `default` arm. Add a `Suspended` variant to `WorkspaceError` and `faber check` fails with `SEM040:missing_variants` at `cli_error_from_workspace` until the boundary chooses a code, copy, and exit code.
- There is no catch-all "unexpected error" branch, because the union has no unknown cases. Broken invariants use `panic`, which exits 70.
- Results go to stdout and errors to stderr, so scripts can pipe output and branch on the exit code.

`faber check` 1.17 reports `WARN003:unused_function` on an `@ conversion` function that is used only by propagation. The conversion does run, as the output above shows.

## Quick Rewrite Table

| Weak | Better |
| --- | --- |
| Something went wrong | We could not save the report. Your edits are still open. Try saving again. |
| Invalid input | Enter a date in MM/DD/YYYY format |
| Failed | Upload failed because the file is larger than 10 MB |
| Network error | You are offline. Reconnect to keep editing shared documents. |
| Access denied | Only workspace admins can invite new members |
| Try again later | The payment provider is unavailable. Try again in a few minutes or use a different payment method. |
