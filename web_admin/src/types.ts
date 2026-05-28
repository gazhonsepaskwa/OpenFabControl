export interface Resource {
  id?: number;
  uuid: string;
  name: string;
  type: string;
  zone: string;
  manual?: string;
  approved?: boolean;
  price_booking_in_eur?: number;
  price_usage_in_eur?: number;
}

export interface Role {
  id: number;
  name: string;
  created_at?: string;
}

export interface User {
  id: number;
  email: string;
  access_key: string;
  password?: string;
  first_name: string;
  last_name: string;
  tva: string;
  facturation_address: string;
  facturation_account: string;
  status: string;
  created_at: string;
  roles?: Role[];
}

export interface Session {
  id: number;
  user_id: number;
  resource_uuid: string;
  started_at: string;
  ended_at: string;
  time_used: number;
  status: string;
}
