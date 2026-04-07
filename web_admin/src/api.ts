/**
 * Thin wrapper around `fetch` that automatically injects the
 * `Authorization: Bearer <token>` header for every request,
 * reading the token from sessionStorage.
 */
export function apiFetch(url: string, options: RequestInit = {}): Promise<Response> {
  const token = sessionStorage.getItem('token');

  const headers = new Headers(options.headers);
  if (token) {
    headers.set('Authorization', `Bearer ${token}`);
  }

  return fetch(url, { ...options, headers });
}
