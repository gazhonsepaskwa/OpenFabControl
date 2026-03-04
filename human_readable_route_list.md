# API Documentation

Error format: `{"error": "message"}` (always JSON).

---

## Machine API (controller)

### POST /machine-api/register
Registers a new machine controller.

**Input (JSON):**
| Parameter | Type   | Required | Description                 |
|-----------|--------|----------|-----------------------------|
| uuid      | string | yes      | Machine UUID                |
| name      | string | yes      | Machine name                |
| type      | string | yes      | Supported type: `fm-bv2`    |

**Outputs:**
| Code | Body                                                                 |
|------|----------------------------------------------------------------------|
| 201  | `{"msg":"registration saved","uuid":"...","name":"..."}`             |
| 400  | `{"error":"invalid payload: ..."}` or `{"error":"UUID already registered"}` |
| 405  | `{"error":"Method not allowed"}`                                     |
| 500  | `{"error":"internal server error"}`                                  |

---

### POST /machine-api/check_approval_status
Checks if a resource is approved.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| uuid      | string | yes      |

**Outputs:**
| Code | Body                                  |
|------|----------------------------------------|
| 200  | `{"approved": true}` or `{"approved": false}` |
| 404  | `{"error":"Resource not found with this UUID"}` |
| 400  | `{"error":"invalid payload: uuid cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`       |
| 500  | `{"error":"Internal Server Error"}`    |

---

### POST /machine-api/create_session
Creates a session (machine side, badge identification).

**Input (JSON):**
| Parameter     | Type   | Required | Description                                      |
|---------------|--------|----------|--------------------------------------------------|
| access_key    | string | yes      | Badge key (linked to a user)                     |
| resource_uuid | string | yes      | Machine UUID                                     |
| started_at    | string | yes      | Scheduled start, RFC3339 (e.g. `2026-02-25T10:00:00Z`) |
| ended_at      | string | no       | Scheduled end, RFC3339. Default: started_at + 30 min |

**Outputs:**
| Code | Body                                                                 |
|------|----------------------------------------------------------------------|
| 201  | `{"session":{id,user_id,resource_uuid,started_at,ended_at,time_used,status}}` |
| 400  | `{"error":"invalid payload: ..."}` or overlap / started_at in the past |
| 404  | `{"error":"User not found with this access_key"}` or resource not found |
| 405  | `{"error":"Method not allowed"}`                                     |
| 500  | `{"error":"Internal Server Error"}`                                  |

---

### POST /machine-api/start_session
Starts a planned session (badge + machine).

**Input (JSON):**
| Parameter     | Type   | Required |
|---------------|--------|----------|
| access_key    | string | yes      |
| resource_uuid | string | yes      |

**Outputs:**
| Code | Body                                                  |
|------|--------------------------------------------------------|
| 200  | `{"session":{id,user_id,resource_uuid,started_at,ended_at,time_used,status:"progress"}}` |
| 404  | `{"error":"User not found with this access_key"}` or `{"error":"No session found to start"}` |
| 400  | `{"error":"invalid payload: ..."}`                     |
| 405  | `{"error":"Method not allowed"}`                       |
| 500  | `{"error":"Internal Server Error"}`                    |

---

### POST /machine-api/stop_session
Stops the current session on a machine.

**Input (JSON):**
| Parameter     | Type   | Required |
|---------------|--------|----------|
| resource_uuid | string | yes      |

**Outputs:**
| Code | Body                                                      |
|------|------------------------------------------------------------|
| 200  | `{"session":{id,user_id,resource_uuid,...,status:"done"}}` |
| 404  | `{"error":"No session in progress for this resource"}`     |
| 400  | `{"error":"invalid payload: resource_uuid cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`                           |
| 500  | `{"error":"Internal Server Error"}`                        |

---

### POST /machine-api/next_booking
Returns the next (or current) booking for a machine. Used by the firmware to display "BOOKED by &lt;user_name&gt;" and time slot. No auth (like start_session).

**Input (JSON):**
| Parameter     | Type   | Required |
|---------------|--------|----------|
| resource_uuid | string | yes      |

**Outputs:**
| Code | Body                                                                 |
|------|----------------------------------------------------------------------|
| 200  | `{"next_booking":{ "start_at":"2025-01-01T15:30:00Z", "end_at":"2025-01-01T17:00:00Z", "user_name":"John Doe" }}` or `{"next_booking":null}` when none |
| 400  | `{"error":"invalid payload: resource_uuid cannot be empty"}`          |
| 405  | `{"error":"Method not allowed"}`                                      |
| 500  | `{"error":"Internal Server Error"}`                                   |

Dates are RFC3339 (firmware uses first 19 chars: `YYYY-MM-DDTHH:MM:SS`). `user_name` is truncated to 32 characters.

---

### POST /machine-api/create_user
Creates a user (same handler as admin).

**Input (JSON):**
| Parameter  | Type   | Required |
|------------|--------|----------|
| access_key | string | yes      |
| email      | string | yes      |

**Outputs:**
| Code | Body                                                       |
|------|------------------------------------------------------------|
| 201  | `{"msg":"user created, email sent","link_sent":"..."}`     |
| 400  | `{"error":"invalid payload: ..."}` or `{"error":"Email already registered"}` |
| 405  | `{"error":"Method not allowed"}`                           |
| 500  | `{"error":"internal server error"}`                        |

---

## Admin API

### Machine controllers

### GET /web-admin-api/get_resource_list_to_approve
Lists machines pending approval.

**Input:** none (GET)

**Outputs:**
| Code | Body                                  |
|------|----------------------------------------|
| 200  | `[{id,uuid,type,zone,name,...},...]`   |
| 405  | `{"error":"Method not allowed"}`       |
| 500  | `{"error":"internal server error"}`    |

---

### GET /web-admin-api/get_resource_list_approved
Lists approved machines.

**Input:** none (GET)

**Outputs:**
| Code | Body                                  |
|------|----------------------------------------|
| 200  | `[{id,uuid,type,zone,name,...},...]`   |
| 405  | `{"error":"Method not allowed"}`       |
| 500  | `{"error":"internal server error"}`    |

---

### POST /web-admin-api/approve_resource
Approves a machine controller.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| uuid      | string | yes      |

**Outputs:**
| Code | Body                                                       |
|------|------------------------------------------------------------|
| 200  | `{"msg":"Machine controler approved successfully"}`        |
| 404  | `{"error":"No device waiting approving with this UUID"}`   |
| 400  | `{"error":"invalid payload: uuid cannot be empty"}`        |
| 405  | `{"error":"Method not allowed"}`                           |
| 500  | `{"error":"Internal Server Error"}`                        |

---

### DELETE /web-admin-api/delete_resource
Deletes a machine controller.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| uuid      | string | yes      |

**Outputs:**
| Code | Body                                                     |
|------|----------------------------------------------------------|
| 200  | `{"msg":"Machine controler deleted successfully"}`       |
| 404  | `{"error":"No device with this UUID registered"}`        |
| 400  | `{"error":"invalid payload: UUID cannot be empty"}`      |
| 405  | `{"error":"Method not allowed"}`                         |
| 500  | `{"error":"Internal Server Error"}`                      |

---

### POST /web-admin-api/edit_resource
Edits a machine controller.

**Input (JSON):**
| Parameter            | Type   | Required | Description                        |
|----------------------|--------|----------|------------------------------------|
| uuid                 | string | yes      | UUID of the machine to edit        |
| zone                 | string | no       | At least one optional field required |
| name                 | string | no       |                                    |
| manual               | string | no       |                                    |
| price_booking_in_eur | string | no       |                                    |
| price_usage_in_eur   | string | no       |                                    |

**Outputs:**
| Code | Body                                                             |
|------|------------------------------------------------------------------|
| 200  | `{"msg":"Machine controler edited successfully"}`                |
| 404  | `{"error":"No device with this UUID registered"}`                |
| 400  | `{"error":"invalid json: no data to update send"}` or other      |
| 405  | `{"error":"Method not allowed"}`                                 |
| 500  | `{"error":"Internal Server Error"}`                              |

---

### POST /web-admin-api/create_session
Creates a session (admin side, user_id identification).

**Input (JSON):**
| Parameter     | Type   | Required | Description                     |
|---------------|--------|----------|---------------------------------|
| user_id       | int    | yes      | User ID                         |
| resource_uuid | string | yes      | Machine UUID                    |
| started_at    | string | yes      | Scheduled start, RFC3339        |
| ended_at      | string | no       | Scheduled end, RFC3339. Default: started_at + 30 min |

**Outputs:** (same as /machine-api/create_session, user_id error instead of access_key)

---

### Users

### POST /web-admin-api/create_user
Same as /machine-api/create_user.

---

### GET /web-admin-api/get_user_list
Lists all users.

**Input:** none (GET)

**Outputs:**
| Code | Body                                                 |
|------|------------------------------------------------------|
| 200  | `[{id,first_name,last_name,tva,...,status,created_at},...]` |
| 405  | `{"error":"Method not allowed"}`                     |
| 500  | `{"error":"Internal server error"}`                  |

---

### DELETE /web-admin-api/delete_user
Deletes a user.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| user_id   | string | yes      |

**Outputs:**
| Code | Body                                          |
|------|------------------------------------------------|
| 200  | `{"msg":"User deleted successfully"}`          |
| 404  | `{"error":"No user with this id saved"}`       |
| 400  | `{"error":"invalid payload: user_id cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`               |
| 500  | `{"error":"Internal Server Error"}`            |

---

### POST /web-admin-api/update_user
Edits a user.

**Input (JSON):**
| Parameter            | Type   | Required | Description                  |
|----------------------|--------|----------|------------------------------|
| id                   | string | yes      | User ID                      |
| access_key           | string | no       | At least one optional field  |
| email                | string | no       |                              |
| first_name           | string | no       |                              |
| last_name            | string | no       |                              |
| tva                  | string | no       |                              |
| facturation_address  | string | no       |                              |
| facturation_account  | string | no       |                              |

**Outputs:**
| Code | Body                                                      |
|------|-----------------------------------------------------------|
| 200  | `{"msg":"User edited successfully"}`                      |
| 404  | `{"error":"No device with this UUID registered"}` (possible inconsistency) |
| 400  | `{"error":"invalid json: no data to update send"}`        |
| 405  | `{"error":"Method not allowed"}`                          |
| 500  | `{"error":"Internal Server Error"}`                       |

---

### POST /web-admin-api/desactivate_user
Deactivates a user.

**Input (JSON):**
| Parameter | Type | Required |
|-----------|------|----------|
| user_id   | int  | yes      |

**Outputs:**
| Code | Body                                                                     |
|------|---------------------------------------------------------------------------|
| 200  | `{"msg":"user successfully desactivated"}`                               |
| 400  | `{"error":"invalid payload: user_id cannot be empty"}` or pending account |
| 405  | `{"error":"Method not allowed"}`                                         |
| 500  | `{"error":"internal server error"}`                                      |

---

### POST /web-admin-api/reactivate_user
Reactivates a user.

**Input (JSON):**
| Parameter | Type | Required |
|-----------|------|----------|
| user_id   | int  | yes      |

**Outputs:**
| Code | Body                                                      |
|------|-----------------------------------------------------------|
| 200  | `{"msg":"user successfully activated"}`                   |
| 400  | `{"error":"invalid payload: user_id cannot be empty"}` etc. |
| 405  | `{"error":"Method not allowed"}`                          |
| 500  | `{"error":"internal server error"}`                       |

---

### POST /web-admin-api/get_user_roles
Returns a user's roles.

**Input (JSON):**
| Parameter | Type | Required |
|-----------|------|----------|
| user_id   | int  | yes      |

**Outputs:**
| Code | Body                                  |
|------|----------------------------------------|
| 200  | `[{id,name,created_at},...]`           |
| 400  | `{"error":"invalid payload: user_id cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`       |
| 500  | `{"error":"internal server error"}`    |

---

### Roles

### GET /web-admin-api/get_role_list
Lists all roles.

**Input:** none (GET)

**Outputs:**
| Code | Body                                  |
|------|----------------------------------------|
| 200  | `[{id,name,created_at},...]`           |
| 405  | `{"error":"Method not allowed"}`       |
| 500  | `{"error":"Internal server error"}`    |

---

### POST /web-admin-api/create_role
Creates a role.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| role_name | string | yes      |

**Outputs:**
| Code | Body                                          |
|------|-----------------------------------------------|
| 200  | `{"msg":"role successfully created"}`         |
| 400  | `{"error":"invalid payload: role_name cannot be empty"}` |
| 409  | `{"error":"role already exists"}`             |
| 405  | `{"error":"Method not allowed"}`              |
| 500  | `{"error":"internal server error"}`           |

---

### DELETE /web-admin-api/delete_role
Deletes a role.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| role_name | string | yes      |

**Outputs:**
| Code | Body                                        |
|------|----------------------------------------------|
| 200  | `{"msg":"Role deleted successfully"}`        |
| 404  | `{"error":"No role with this name saved"}`   |
| 400  | `{"error":"invalid payload: role_name cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`             |
| 500  | `{"error":"Internal Server Error"}`          |

---

### POST /web-admin-api/assign_role_to_user
Assigns a role to a user.

**Input (JSON):**
| Parameter | Type | Required |
|-----------|------|----------|
| user_id   | int  | yes      |
| role_id   | int  | yes      |

**Outputs:**
| Code | Body                                                       |
|------|------------------------------------------------------------|
| 200  | `{"msg":"role with id X successfully added to user with id Y"}` |
| 400  | `{"error":"invalid payload: user_id/role_id cannot be empty"}` |
| 500  | `{"error":"This role is already assigned to this user"}` or server error |

---

### DELETE /web-admin-api/remove_role_from_user
Removes a role from a user.

**Input (JSON):**
| Parameter | Type | Required |
|-----------|------|----------|
| user_id   | int  | yes      |
| role_id   | int  | yes      |

**Outputs:**
| Code | Body                                                      |
|------|-----------------------------------------------------------|
| 200  | `{"msg":"Relation deleted"}`                              |
| 400  | `{"error":"invalid payload: user_id/role_id cannot be empty"}` |
| 500  | `{"error":"This relation does not exist"}` or server error |

---

## User API

### POST /web-user-api/user_one_time_setup
Initial account setup (one-time only).

**Input (JSON):**
| Parameter           | Type   | Required |
|---------------------|--------|----------|
| activation_code     | string | yes      |
| first_name          | string | yes      |
| last_name           | string | yes      |
| password            | string | yes      |
| tva                 | string | yes      |
| facturation_address | string | yes      |
| facturation_account | string | yes      |

**Outputs:**
| Code | Body                                                        |
|------|-------------------------------------------------------------|
| 201  | `{"msg":"user set-up, you can login now"}`                  |
| 400  | `{"error":"invalid payload: ..."}` or `{"error":"Invalid activation code"}` or `{"error":"Account already set-up"}` |
| 405  | `{"error":"Method not allowed"}`                            |
| 500  | `{"error":"Internal Server Error"}`                         |

---

### POST /web-user-api/create_session
Creates a session (user side, user_id identification). Same inputs/outputs as admin create_session.

---

### POST /web-user-api/login
Login with credentials.

**Input (JSON):**
| Parameter | Type   | Required |
|-----------|--------|----------|
| email     | string | yes      |
| password  | string | yes      |

**Outputs:**
| Code | Body                                                  |
|------|--------------------------------------------------------|
| 200  | `{"msg":"logged in successfully","token":"<JWT>"}`     |
| 403  | `{"error":"Invalid credential"}`                       |
| 400  | `{"error":"invalid payload: email/password cannot be empty"}` |
| 405  | `{"error":"Method not allowed"}`                       |
| 500  | Internal server error                                  |

---

## TODO (routes to implement)

### Users
- POST /web-user-api/me
- POST /web-user-api/edit_profile
- POST /web-admin-api/login
- POST /web-user-api/logout
- POST /web-admin-api/logout

### Machine controllers
- POST /machine-api/update_time_used
