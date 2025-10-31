#ifndef METADATA_H
#define METADATA_H

// Initializes the metadata management system (e.g., lock table).
void metadata_init();

// Cleans up and destroys the metadata management system.
void metadata_destroy();

// Acquires a lock for a given file path to ensure atomic operations.
void metadata_lock_file(const char* path);

// Releases the lock for a given file path.
void metadata_unlock_file(const char* path);

// Checks the version of a file against a client's version (for conflict detection).
// Returns 1 if the client has the latest version, 0 otherwise.
int metadata_check_version(const char* path, int client_version);

// Updates the version of a file after a write operation.
void metadata_update_version(const char* path);

#endif // METADATA_H
