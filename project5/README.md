Your program must include appropriate logic to handle exceptional cases and errors, including:
1. invalid or missing command line arguments
2. target directory does not exist
3. manifest file cannot be opened or is missing in check mode
4. malformed lines in the manifest file (print a clear error and halt)
5. file read failures during scanning (do not halt, but record the issue
