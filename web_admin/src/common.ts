export const ADMIN_ROLE_ID = 1;

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
