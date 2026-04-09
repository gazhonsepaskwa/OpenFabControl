export const ADMIN_ROLE_ID = 1;

export function getBaseUrl(): string {
  return import.meta.env.DEV ? `https://${window.location.hostname}:4080` : '';
}

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

  return fetch(`${getBaseUrl()}${url}`, { ...options, headers });
}

/** Returns true if the user stored in sessionStorage has the admin role. */
export function isAdmin(): boolean {
  try {
    const user = JSON.parse(sessionStorage.getItem('user') ?? 'null');
    return Array.isArray(user?.roles) && user.roles.some((r: { id: number }) => r.id === ADMIN_ROLE_ID);
  } catch {
    return false;
  }
}

/** Returns the numeric ID of the user stored in sessionStorage, or null if unavailable. */
export function getCurrentUserId(): number | null {
  try {
    const user = JSON.parse(sessionStorage.getItem('user') ?? 'null');
    return typeof user?.id === 'number' ? user.id : null;
  } catch {
    return null;
  }
}
