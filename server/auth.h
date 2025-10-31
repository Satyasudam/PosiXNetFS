#ifndef AUTH_H
#define AUTH_H

/**
 * @brief Authenticates a user based on username and password.
 * * @param username The user's name.
 * @param password The user's password.
 * @return int Returns 1 on successful authentication, 0 on failure.
 */
int auth_user(const char* username, const char* password);

#endif // AUTH_H
