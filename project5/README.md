Your program must include appropriate logic to handle exceptional cases and errors, including:
Good enough, i should check to make sure that -m and -d and -r are followed 
up with a valid string and not "" or a command 
-- 1. invalid or missing command line arguments
DONE 2. target directory does not exist
DONE 3. manifest file cannot be opened or is missing in check mode
DONE 4. malformed lines in the manifest file (print a clear error and halt)
DONE 5. file read failures during scanning (do not halt, but record the issue
