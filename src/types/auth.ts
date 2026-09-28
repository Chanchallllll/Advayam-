export type UserRole = "developer" | "user";

export type RegisterStep = 1 | 2 | 3;

export type RegisterFormData = {
  role: UserRole;
  emailVerified: boolean;
  profileCompleted: boolean;
};